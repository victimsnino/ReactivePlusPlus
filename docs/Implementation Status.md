# Implementation status

## Foundamentals
- [x] Observables
  - [x] Specific Observable
  - [x] Dynamic Observable
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
  - [ ] RunLoop
  - [ ] EventLoop

## Creating Observables
- [x] Create
- [x] Just 
  - [x] + memory_model
- [x] empty/never/error
- [ ] from
  - [ ] variadic
  - [ ] iterable
  - [ ] future
  - [ ] promise
  - [ ] callback
  - [ ] async
- [ ] defer
- [ ] interval
- [ ] range
- [ ] repeat
- [ ] timer

## Operators
### Transforming
- [x] map

### Filtering
- [x] filter
- [x] take
