window.BENCHMARK_DATA = {
  "lastUpdate": 1643747226764,
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
      },
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
          "id": "37679379dc8c1b0453110b21cc2bbba6dd09b9d0",
          "message": "Split observables to specific and dynamic (#12)\n\n* Split observable to two types\r\n\r\n* make specific and dynamic observables\r\n\r\n* nodiscard\r\n\r\n* fix vdtor\r\n\r\n* compile fix",
          "timestamp": "2022-02-01T23:22:33+03:00",
          "tree_id": "666496a10eb071acffffc577071796aa0e17386e",
          "url": "https://github.com/victimsnino/ReactivePlusPlus/commit/37679379dc8c1b0453110b21cc2bbba6dd09b9d0"
        },
        "date": 1643747017171,
        "tool": "catch2",
        "benches": [
          {
            "name": "Construction",
            "value": 57.0422,
            "range": "± 2.33079",
            "unit": "ns",
            "extra": "100 samples\n527 iterations"
          },
          {
            "name": "Subscribe",
            "value": 28.1078,
            "range": "± 0.187845",
            "unit": "ns",
            "extra": "100 samples\n1060 iterations"
          },
          {
            "name": "OnNext",
            "value": 2.0101,
            "range": "± 0.0291338",
            "unit": "ns",
            "extra": "100 samples\n14873 iterations"
          },
          {
            "name": "Construction",
            "value": 86.673,
            "range": "± 3.76649",
            "unit": "ns",
            "extra": "100 samples\n348 iterations"
          },
          {
            "name": "Subscribe",
            "value": 28.4811,
            "range": "± 0.679272",
            "unit": "ns",
            "extra": "100 samples\n1046 iterations"
          },
          {
            "name": "OnNext",
            "value": 1.6751,
            "range": "± 0.0265173",
            "unit": "ns",
            "extra": "100 samples\n17845 iterations"
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
          "id": "d69e74295788597dc7aedc6841f8b08d41dc65d9",
          "message": "fix tests",
          "timestamp": "2022-02-01T23:26:08+03:00",
          "tree_id": "2e22ac6344a9fce0d23d53e5fa73aba094637c5e",
          "url": "https://github.com/victimsnino/ReactivePlusPlus/commit/d69e74295788597dc7aedc6841f8b08d41dc65d9"
        },
        "date": 1643747226298,
        "tool": "catch2",
        "benches": [
          {
            "name": "Specific observable construction",
            "value": 63.1522,
            "range": "± 2.21162",
            "unit": "ns",
            "extra": "100 samples\n502 iterations"
          },
          {
            "name": "Dynamic observable construction",
            "value": 96.0096,
            "range": "± 3.94178",
            "unit": "ns",
            "extra": "100 samples\n361 iterations"
          },
          {
            "name": "Specific observable subscribe",
            "value": 29.0497,
            "range": "± 0.574937",
            "unit": "ns",
            "extra": "100 samples\n1034 iterations"
          },
          {
            "name": "Dynamic observable subscribe",
            "value": 28.557,
            "range": "± 0.593631",
            "unit": "ns",
            "extra": "100 samples\n1060 iterations"
          },
          {
            "name": "OnNext",
            "value": 2.01093,
            "range": "± 0.048846",
            "unit": "ns",
            "extra": "100 samples\n14853 iterations"
          }
        ]
      }
    ]
  }
}