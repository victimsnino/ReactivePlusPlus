# Exception guarantee {#exception_guarantee} 

## Overview

In non-reactive programming functions/modules throws exception in case of something invalid. As a result, user can catch it and handle it somehow while internal state of objects can be in some state (invalid/untouched/partly valid) and etc.

In reactive programming there is another way of exception mechanism: throwing exception as is from original place is useless. Notification about "something goes wrong" need to receive observer/subscriber, not owner of callstack. As a result, ANY exception obtained during emitting items and etc WOULD be delivered to subscriber/observer via `on_error` function and then unsubscribe happens. As a result, no any raw exceptions would be throws during using RPP. In case of emitting `on_error` whole internal state of observable keeps valid but it doesn't matter - whole chain would be destroyed due to `on_error` forces unsubscribe. Reactive catching mechanisms like `catch` or `retry` **re-subscribes** on observable. it means, that new chain with new states would be created, not re-used existing one.
