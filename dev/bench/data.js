window.BENCHMARK_DATA = {
  "lastUpdate": 1642282171912,
  "repoUrl": "https://github.com/victimsnino/ReactivePlusPlus",
  "entries": {
    "Catch2 Benchmark Linux CLANG": [
      {
        "commit": {
          "author": {
            "email": "32845901+victimsnino@users.noreply.github.com",
            "name": "Aleksey Loginov",
            "username": "victimsnino"
          },
          "committer": {
            "email": "noreply@github.com",
            "name": "GitHub",
            "username": "web-flow"
          },
          "distinct": true,
          "id": "314e49c2a22a537a1f5f59556f4ad2e979734ade",
          "message": "Debug bench (#11)",
          "timestamp": "2022-01-15T22:49:41+03:00",
          "tree_id": "71bacf76fb16fb2698e890b6549aa473d461768d",
          "url": "https://github.com/victimsnino/ReactivePlusPlus/commit/314e49c2a22a537a1f5f59556f4ad2e979734ade"
        },
        "date": 1642282171046,
        "tool": "catch2",
        "benches": [
          {
            "name": "Construction",
            "value": 91.6263,
            "range": "± 27.3095",
            "unit": "ns",
            "extra": "100 samples\n329 iterations"
          },
          {
            "name": "Subscribe",
            "value": 33.1601,
            "range": "± 11.3824",
            "unit": "ns",
            "extra": "100 samples\n962 iterations"
          },
          {
            "name": "OnNext",
            "value": 2.27589,
            "range": "± 1.28448",
            "unit": "ns",
            "extra": "100 samples\n15568 iterations"
          }
        ]
      }
    ]
  }
}