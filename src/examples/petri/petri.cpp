// This is a small example in which a 'petri net executor' is implemented using Reactive
// Streams. Petri nets can be seen as a generalization of state machines and are
// particulary effective for modeling concurrent programs. See
// https://en.wikipedia.org/wiki/Petri_net for a brief introduction to Petri nets.

#include <rpp/rpp.hpp>

#include <iostream>

using Transition  = int;
using Transitions = std::vector<Transition>;
using Place       = char;
using Places      = std::vector<Place>;
struct Mutations
{
    Places consume, produce;
};
using PetriNet = std::map<Transition, Mutations>;
using Marking  = std::map<Place, int>;
typedef void (*const TransitionFunction)(void);

using namespace std::ranges;

void foo() { std::cout << "foo" << std::endl; }
void bar() { std::cout << "bar" << std::endl; }

// The state of a system is determined by its Petri Net (constant) and its marking (changes)
const PetriNet net = {{0, {{'a', 'b'}, {'c'}}}, {1, {{'c', 'c'}, {'b', 'b', 'd'}}}};
const Marking  marking_start = {{'a', 4}, {'b', 2}, {'c', 0}, {'d', 0}};

const std::map<Transition, TransitionFunction> store = {{0, &foo}, {1, &bar}};

const auto& run_transition(Transition t)
{
    store.at(t)();            // calls the function associated with the transition
    return net.at(t).produce; // contains the post marking mutations.
}

std::pair<Marking, Transitions> mutate_marking(Marking&& marking, const Places& produce)
{
    // Produce tokens
    for_each(produce, [&marking](const auto& place) { marking[place] += 1; });
    // Consume tokens by exeucting enabled transitions
    Transitions transitions;
    for (const auto& [transition, mutations] : net)
    {
        const auto& c = mutations.consume;
        if (!c.empty() &&
            all_of(c, [&](const auto& place) { return marking[place] >= count(c, place); }))
        {
            for_each(c, [&marking](const auto& place) { marking[place] -= 1; });
            transitions.push_back(transition);
        }
    }

    return std::make_pair(marking, transitions);
}

int main()
{
    // A subject is used to create a stream of 'transitions' that should be 'executed'.
    const auto transitions_subject = rpp::subjects::publish_subject<Transition>{};

    // In our case, we have a finite program and we expect the program to end up in a
    // particular marking. Hence we create a predicate that compares the marking to the
    // expected (end) marking.
    const Marking marking_end   = {{'a', 0}, {'b', 2}, {'c', 0}, {'d', 2}};
    const auto    end_predicate = [=](const auto& marking)
    { return !std::equal(marking.begin(), marking.end(), marking_end.begin()); };

    // Reactive streams lend themselves very nicely to create a 'petri net executor':
    transitions_subject.get_observable()
        .map(&run_transition)
        .start_with(Places{}) // we start with an empty set
        .observe_on(rpp::schedulers::trampoline{})
        .scan(marking_start,
              [&transitions_subject](auto&& marking, const auto& places)
              {
                  Transitions transitions;
                  std::tie(marking, transitions) = mutate_marking(std::move(marking), places);
                  for_each(transitions,
                           [dispatcher = transitions_subject.get_subscriber()](const auto& t)
                           { dispatcher.on_next(t); });
                  return marking;
              })
        .take_while(end_predicate)
        .subscribe([](const auto&) {}, [] { std::cout << "done" << std::endl; });

    return 0;
}