window.BENCHMARK_DATA = {
  "lastUpdate": 1643832580302,
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
      },
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
          "id": "1f4aebc3b74671dd4d8f87c844625a2704d16ef6",
          "message": "Test for dynamic_observable + as_dynamic ref_qualifiers",
          "timestamp": "2022-02-02T23:08:17+03:00",
          "tree_id": "2479729448b37c564117219bc70035a207c58c4e",
          "url": "https://github.com/victimsnino/ReactivePlusPlus/commit/1f4aebc3b74671dd4d8f87c844625a2704d16ef6"
        },
        "date": 1643832579392,
        "tool": "catch2",
        "benches": [
          {
            "name": "Specific observable construction",
            "value": 61.5565,
            "range": "± 4.09312",
            "unit": "ns",
            "extra": "100 samples\n482 iterations"
          },
          {
            "name": "Dynamic observable construction",
            "value": 100.535,
            "range": "± 37.347",
            "unit": "ns",
            "extra": "100 samples\n294 iterations"
          },
          {
            "name": "Specific observable subscribe",
            "value": 28.1187,
            "range": "± 5.31732",
            "unit": "ns",
            "extra": "100 samples\n920 iterations"
          },
          {
            "name": "Dynamic observable subscribe",
            "value": 31.574,
            "range": "± 30.6993",
            "unit": "ns",
            "extra": "100 samples\n997 iterations"
          },
          {
            "name": "OnNext",
            "value": 1.73988,
            "range": "± 0.262411",
            "unit": "ns",
            "extra": "100 samples\n13893 iterations"
          }
        ]
      }
    ]
  }
}