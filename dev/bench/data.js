window.BENCHMARK_DATA = {
  "lastUpdate": 1643749589199,
  "repoUrl": "https://github.com/victimsnino/ReactivePlusPlus",
  "entries": {
    "Catch2 Benchmark Linux CLANG": [
      {
        "commit": {
          "author": {
            "email": "victimsnino@gmail.com",
            "name": "Aleksey Loginov",
            "username": "victimsnino"
          },
          "committer": {
            "email": "victimsnino@gmail.com",
            "name": "Aleksey Loginov",
            "username": "victimsnino"
          },
          "distinct": true,
          "id": "1de7540c73e7662c5521cd36b67611078327774f",
          "message": "Remove universal ref",
          "timestamp": "2022-02-02T00:04:39+03:00",
          "tree_id": "c5d0a3be07bb665797e66f4b35eacca548546f34",
          "url": "https://github.com/victimsnino/ReactivePlusPlus/commit/1de7540c73e7662c5521cd36b67611078327774f"
        },
        "date": 1643749588323,
        "tool": "catch2",
        "benches": [
          {
            "name": "Specific observable construction",
            "value": 58.3696,
            "range": "± 4.2066",
            "unit": "ns",
            "extra": "100 samples\n521 iterations"
          },
          {
            "name": "Dynamic observable construction",
            "value": 83.881,
            "range": "± 4.2361",
            "unit": "ns",
            "extra": "100 samples\n307 iterations"
          },
          {
            "name": "Specific observable subscribe",
            "value": 28.2184,
            "range": "± 0.766543",
            "unit": "ns",
            "extra": "100 samples\n1042 iterations"
          },
          {
            "name": "Dynamic observable subscribe",
            "value": 28.0745,
            "range": "± 0.749636",
            "unit": "ns",
            "extra": "100 samples\n1054 iterations"
          },
          {
            "name": "OnNext",
            "value": 2.01147,
            "range": "± 0.0385363",
            "unit": "ns",
            "extra": "100 samples\n14873 iterations"
          }
        ]
      }
    ]
  }
}