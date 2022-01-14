window.BENCHMARK_DATA = {
  "lastUpdate": 1642196260045,
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
          "id": "e0575b5afe7ebfccac9fcd04bfcede0fcbbcea75",
          "message": "Debug bench",
          "timestamp": "2022-01-14T20:51:51Z",
          "url": "https://github.com/victimsnino/ReactivePlusPlus/pull/11/commits/e0575b5afe7ebfccac9fcd04bfcede0fcbbcea75"
        },
        "date": 1642196256483,
        "tool": "catch2",
        "benches": [
          {
            "name": "Construction",
            "value": 160.623,
            "range": "± 31.6275",
            "unit": "ns",
            "extra": "100 samples\n172 iterations"
          },
          {
            "name": "Subscribe",
            "value": 24.5711,
            "range": "± 2.30262",
            "unit": "ns",
            "extra": "100 samples\n1121 iterations"
          },
          {
            "name": "OnNext",
            "value": 0.590368,
            "range": "± 0.202631",
            "unit": "ns",
            "extra": "100 samples\n53816 iterations"
          },
          {
            "name": "Raw call to lambda",
            "value": 0.491646,
            "range": "± 0.139807",
            "unit": "ns",
            "extra": "100 samples\n58752 iterations"
          },
          {
            "name": "Call to lambda via std::function",
            "value": 1.88327,
            "range": "± 0.631036",
            "unit": "ns",
            "extra": "100 samples\n16481 iterations"
          },
          {
            "name": "Copy  lambda",
            "value": 22.125,
            "range": "± 5.97137",
            "unit": "ns",
            "extra": "100 samples\n1366 iterations"
          },
          {
            "name": "Copy  std::function",
            "value": 37.4258,
            "range": "± 2.55007",
            "unit": "ns",
            "extra": "100 samples\n704 iterations"
          },
          {
            "name": "Make shared  copy of lambda",
            "value": 36.0818,
            "range": "± 2.44616",
            "unit": "ns",
            "extra": "100 samples\n719 iterations"
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
          "id": "e0575b5afe7ebfccac9fcd04bfcede0fcbbcea75",
          "message": "Debug bench",
          "timestamp": "2022-01-14T20:51:51Z",
          "url": "https://github.com/victimsnino/ReactivePlusPlus/pull/11/commits/e0575b5afe7ebfccac9fcd04bfcede0fcbbcea75"
        },
        "date": 1642196259150,
        "tool": "catch2",
        "benches": [
          {
            "name": "Construction",
            "value": 112.225,
            "range": "± 9.4648",
            "unit": "ns",
            "extra": "100 samples\n332 iterations"
          },
          {
            "name": "Subscribe",
            "value": 30.6051,
            "range": "± 4.01891",
            "unit": "ns",
            "extra": "100 samples\n1039 iterations"
          },
          {
            "name": "OnNext",
            "value": 2.0109,
            "range": "± 0.0283315",
            "unit": "ns",
            "extra": "100 samples\n18230 iterations"
          },
          {
            "name": "Raw call to lambda",
            "value": 0.776624,
            "range": "± 0.0975566",
            "unit": "ns",
            "extra": "100 samples\n45118 iterations"
          },
          {
            "name": "Call to lambda via std::function",
            "value": 2.05248,
            "range": "± 0.237658",
            "unit": "ns",
            "extra": "100 samples\n18268 iterations"
          },
          {
            "name": "Copy  lambda",
            "value": 8.31594,
            "range": "± 0.898642",
            "unit": "ns",
            "extra": "100 samples\n3870 iterations"
          },
          {
            "name": "Copy  std::function",
            "value": 31.2605,
            "range": "± 4.33114",
            "unit": "ns",
            "extra": "100 samples\n1093 iterations"
          },
          {
            "name": "Make shared  copy of lambda",
            "value": 26.9335,
            "range": "± 4.34647",
            "unit": "ns",
            "extra": "100 samples\n1253 iterations"
          }
        ]
      }
    ]
  }
}