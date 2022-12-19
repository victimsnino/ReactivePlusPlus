# Implementation status {#status}

## Fundamentals

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
  - [x] CurrentThread/Trampoline
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
- [x] interval
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
  - [x] count
    - [ ] skip
  - [ ] time
  - [ ] time_or_count
- [ ] window
  - [x] count
    - [ ] skip
  - [ ] time
  - [ ] time_or_count

### Filtering
- [x] filter
- [x] take
- [x] debounce
- [ ] distinct
  - [ ] distinct
  - [x] distinct_until_changed
- [ ] element_at
- [x] first
- [ ] ignore_elements
- [x] last
- [ ] sample
  - [ ] sample (observable)
  - [x] sample_with_time
- [x] skip
- [ ] skip_last
- [x] take_last

### Conditional

- [x] take_while
- [ ] all
- [ ] amb
- [ ] contains
- [ ] default_if_empty
- [ ] sequence_equal
- [ ] skip_until
- [ ] skip_while
- [x] take_until

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
- [x] combine_latest
- [ ] zip

### Aggregate

- [x] average
- [x] concat
- [ ] count
- [ ] max 
- [ ] min
- [x] reduce
- [ ] sum

### Backpressure

- [ ] backpressure ???

### Error handling
- [x] catch
- [ ] retry

### Utility

- [x] observe_on
- [x] repeat
  - [ ] scheduling (by default trampoline ?)
- [x] subscribe_on
- [x] delay
- [x] do/tap
  - [x] tap with observer
  - [x] tap with callbacks
  - [x] do_on_next
  - [x] do_on_error
  - [x] do_on_completed
- [x] timeout 
  - [x] timeout
  - [x] timeout with fallback observable

### Connectable

- [x] publish
- [x] multicast
- [x] connect
- [x] ref_count
- [ ] replay

## Subjects

- [x] publish_subject
- [x] behavior_subject
- [ ] serialized_subject
- [ ] replay_subject
- [ ] async_subject
