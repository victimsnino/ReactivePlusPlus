# Implementation status {#status}

## Fundamentals

- [ ] Observables
  - [x] Specific Observable
  - [ ] Dynamic Observable
  - [ ] Blocking Observable
  - [ ] Connectable Observable
- [x] Observers
  - [x] Specific Observer
  - [x] Dynamic Observer
- [ ] Disposables
  - [x] Base Disposable
  - [ ] Callback Disposable
- [ ] Schedulers
  - [x] Immediate
  - [ ] New Thread
  - [x] CurrentThread
  - [?] Trampoline
  - [ ] RunLoop
  - [ ] EventLoop

## Creating Observables

- [x] Create
- [x] Just
  - [x] + memory_model
- [ ] empty/never/error
- [ ] from
  - [x] iterable
  - [ ] future
  - [ ] promise
  - [ ] callable
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
- [ ] flat_map
- [ ] scan
- [ ] buffer
  - [ ] count
    - [ ] skip
  - [ ] time
  - [ ] time_or_count
- [ ] window
  - [ ] count
    - [ ] skip
  - [ ] time
  - [ ] time_or_count

### Filtering
- [x] filter
- [x] take
- [ ] debounce
- [ ] distinct
  - [ ] distinct
  - [ ] distinct_until_changed
- [ ] element_at
- [ ] first
- [ ] ignore_elements
- [ ] last
- [ ] sample
  - [ ] sample (observable)
  - [ ] sample_with_time
- [ ] skip
- [ ] skip_last
- [ ] take_last

### Conditional

- [ ] take_while
- [ ] all
- [ ] amb
- [ ] contains
- [ ] default_if_empty
- [ ] sequence_equal
- [ ] skip_until
- [ ] skip_while
- [ ] take_until

### Combining

- [ ] merge
  - [ ] observable of observables
  - [ ] merge with
  - [ ] merge delay error
- [ ] switch
  - [ ] switch_map
  - [ ] switch_on_next
  - [ ] switch_if_empty
- [ ] with_latest_from
- [ ] start_with
- [ ] combine_latest
- [ ] zip

### Aggregate

- [ ] average
- [ ] concat
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

- [ ] observe_on
- [ ] repeat
  - [ ] scheduling (by default trampoline ?)
- [ ] subscribe_on
- [ ] delay
- [ ] do/tap
  - [ ] tap with observer
  - [ ] tap with callbacks
  - [ ] do_on_next
  - [ ] do_on_error
  - [ ] do_on_completed
- [ ] timeout
  - [ ] timeout
  - [ ] timeout with fallback observable

### Connectable

- [ ] publish
- [ ] multicast
- [ ] connect
- [ ] ref_count
- [ ] replay

## Subjects

- [ ] publish_subject
- [ ] behavior_subject
- [ ] serialized_subject
- [ ] replay_subject
- [ ] async_subject
