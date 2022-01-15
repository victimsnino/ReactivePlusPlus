window.BENCHMARK_DATA = {
  "lastUpdate": 1642275416344,
  "repoUrl": "https://github.com/victimsnino/ReactivePlusPlus",
  "entries": {
    "Catch2 Benchmark Linux GCC": [
      {
        "commit": {
          "author": {
            "name": "victimsnino",
            "username": "victimsnino"
          },
          "committer": {
            "name": "victimsnino",
            "username": "victimsnino"
          },
          "id": "f6c13f5153ee7cdb2ff4bdbfbfccd47383af0127",
          "message": "Debug bench",
          "timestamp": "2022-01-14T20:51:51Z",
          "url": "https://github.com/victimsnino/ReactivePlusPlus/pull/11/commits/f6c13f5153ee7cdb2ff4bdbfbfccd47383af0127"
        },
        "date": 1642275414490,
        "tool": "catch2",
        "benches": [
          {
            "name": "Construction",
            "value": 200.192,
            "range": "± 30.9308",
            "unit": "ns",
            "extra": "100 samples\n146 iterations"
          },
          {
            "name": "Subscribe",
            "value": 33.0834,
            "range": "± 12.2405",
            "unit": "ns",
            "extra": "100 samples\n948 iterations"
          },
          {
            "name": "OnNext",
            "value": 0.61675,
            "range": "± 0.20919",
            "unit": "ns",
            "extra": "100 samples\n46557 iterations"
          }
        ]
      }
    ],
    "Catch2 Benchmark Linux CLANG": [
      {
        "commit": {
          "author": {
            "name": "victimsnino",
            "username": "victimsnino"
          },
          "committer": {
            "name": "victimsnino",
            "username": "victimsnino"
          },
          "id": "f6c13f5153ee7cdb2ff4bdbfbfccd47383af0127",
          "message": "Debug bench",
          "timestamp": "2022-01-14T20:51:51Z",
          "url": "https://github.com/victimsnino/ReactivePlusPlus/pull/11/commits/f6c13f5153ee7cdb2ff4bdbfbfccd47383af0127"
        },
        "date": 1642275415257,
        "tool": "catch2",
        "benches": [
          {
            "name": "Construction",
            "value": 98.4961,
            "range": "± 1.49193",
            "unit": "ns",
            "extra": "100 samples\n329 iterations"
          },
          {
            "name": "Subscribe",
            "value": 32.4846,
            "range": "± 0.782916",
            "unit": "ns",
            "extra": "100 samples\n1026 iterations"
          },
          {
            "name": "OnNext",
            "value": 1.93818,
            "range": "± 0.0542917",
            "unit": "ns",
            "extra": "100 samples\n15832 iterations"
          }
        ]
      }
    ]
  }
}