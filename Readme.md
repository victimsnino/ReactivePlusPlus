## Benchmark for observer:

Naive std::function

Construction                                   100           119     2.5942 ms
                                        252.655 ns    248.126 ns    264.445 ns
                                        34.6542 ns    14.7171 ns    71.6909 ns

Subscribe                                      100           248     2.5544 ms
                                        103.371 ns    102.097 ns    107.956 ns
                                        10.8454 ns    2.76139 ns    24.6666 ns

Storage with casts:
Construction                                   100           183     2.6718 ms
                                        141.366 ns    140.585 ns    143.372 ns
                                        5.74203 ns    1.44133 ns    11.1024 ns

Subscribe                                      100          1429     2.5722 ms
                                        20.9755 ns    20.8251 ns    21.6081 ns
                                        1.35184 ns   0.229986 ns    3.16807 ns