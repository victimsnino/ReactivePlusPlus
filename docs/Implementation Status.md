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
- [x] group_by
- [x] flat_map
- [x] scan
- [ ] buffer
- [ ] window

### Filtering
- [x] filter
- [x] take
- [ ] debounce
- [ ] distinct
- [ ] element_at
- [ ] first
- [ ] ignore_elements
- [ ] last
- [ ] sample
- [ ] skip
- [ ] skip_last
- [ ] take_last

### Conditional

- [x] take_while
- [ ] all
- [ ] amb
- [ ] contains
- [ ] default_if_empty
- [ ] sequence_equal
- [ ] skip_until
- [ ] skip_while
- [ ] take_until
### Combining

- [x] merge
  - [x] observable of observables
  - [x] merge with
  - [ ] merge delay error
- [ ] switch
  - [x] switch_map
  - [x] switch_on_next
  - [ ] switch_if_empty
- [x] with_latest_from
- [x] start_with
- [ ] combine_latest
- [ ] zip

### Aggregate

- [ ] average
- [x] concat
- [ ] count
- [ ] max 
- [ ] min
- [ ] reduce
- [ ] sum

### Backpressure

- [ ] backpressure ???

### Error handling
- [ ] catch
- [ ] retry

### Utility

- [x] observe_on
- [x] repeat
  - [ ] scheduling (by default current_thread ?)
- [x] subscribe_on
- [ ] delay
- [ ] do/tap
- [ ] timeout
- [ ] 
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