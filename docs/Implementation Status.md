# Implementation status {#status}

## Foundamentals

- [x] Observables
  - [x] Specific Observable
  - [x] Dynamic Observable
  - [x] Blocking Observable
  - [x] Connectable Observable
- [x] Observers
  - [x] Specific Observer
  - [x] Dynamic Observer
- [x] Subscribers
  - [x] Specific Subscriber
  - [x] Dynamic Subscriber
- [x] Subscriptions
  - [x] Composite Subscription
  - [x] Callback Subscription 
- [ ] Schedulers
  - [x] Immediate
  - [x] New Thread
  - [ ] CurrentThread/Trampoline
  - [x] RunLoop
  - [ ] EventLoop

## Creating Observables

- [x] Create
- [x] Just 
  - [x] + memory_model
- [x] empty/never/error
- [ ] from
  - [x] iterable
  - [ ] future
  - [ ] promise
  - [x] callable
  - [ ] async
- [ ] defer
- [ ] interval
- [ ] range
- [ ] repeat
- [ ] timer

## Operators
### Transforming

- [x] map
- [ ] group_by
- [ ] window

### Filtering
- [x] filter
- [x] take

### Conditional

- [x] take_while
### Combining

- [x] merge
  - [x] observable of observables
  - [x] merge with
  - [ ] merge delay error
- [ ] switch
  - [ ] switch_on_next
  - [ ] switch_if_empty

### Utility

- [x] observe_on
- [x] repeat
- [x] subscribe_on
### Connectable

- [x] publish
- [x] multicast
- [x] connect
- [x] ref_count
- [ ] replay


## Subjects

- [x] publish_subject
- [ ] serialized_subject
- [ ] replay_subject
- [ ] publish_subject
- [ ] async_subject
