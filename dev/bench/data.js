window.BENCHMARK_DATA = {
  "lastUpdate": 1645127007282,
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
          "id": "5109d8be0d95ad94276159a6bcb0a63082671faf",
          "message": "compile fix",
          "timestamp": "2022-02-03T00:05:19+03:00",
          "tree_id": "75847132422d032ede2da40c18fa15941b95ecc8",
          "url": "https://github.com/victimsnino/ReactivePlusPlus/commit/5109d8be0d95ad94276159a6bcb0a63082671faf"
        },
        "date": 1643835989259,
        "tool": "catch2",
        "benches": [
          {
            "name": "Specific observable construction",
            "value": 60.2112,
            "range": "± 4.1052",
            "unit": "ns",
            "extra": "100 samples\n519 iterations"
          },
          {
            "name": "Dynamic observable construction",
            "value": 82.2395,
            "range": "± 4.00495",
            "unit": "ns",
            "extra": "100 samples\n370 iterations"
          },
          {
            "name": "Specific observable subscribe",
            "value": 25.2586,
            "range": "± 0.47841",
            "unit": "ns",
            "extra": "100 samples\n1170 iterations"
          },
          {
            "name": "Dynamic observable subscribe",
            "value": 25.4342,
            "range": "± 0.385639",
            "unit": "ns",
            "extra": "100 samples\n1176 iterations"
          },
          {
            "name": "OnNext",
            "value": 1.6734,
            "range": "± 0.00842298",
            "unit": "ns",
            "extra": "100 samples\n17852 iterations"
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
          "id": "44c585d5b4360f3942f192a72aac23bdc2369560",
          "message": "small fixes",
          "timestamp": "2022-02-03T00:16:33+03:00",
          "tree_id": "cdbaba3d31af44629f7b28f59d40ef330e2fe460",
          "url": "https://github.com/victimsnino/ReactivePlusPlus/commit/44c585d5b4360f3942f192a72aac23bdc2369560"
        },
        "date": 1643836655982,
        "tool": "catch2",
        "benches": [
          {
            "name": "Specific observable construction",
            "value": 65.5503,
            "range": "± 2.29424",
            "unit": "ns",
            "extra": "100 samples\n486 iterations"
          },
          {
            "name": "Dynamic observable construction",
            "value": 88.4646,
            "range": "± 7.69443",
            "unit": "ns",
            "extra": "100 samples\n300 iterations"
          },
          {
            "name": "Specific observable subscribe",
            "value": 24.8476,
            "range": "± 0.375815",
            "unit": "ns",
            "extra": "100 samples\n1165 iterations"
          },
          {
            "name": "Dynamic observable subscribe",
            "value": 25.525,
            "range": "± 0.409434",
            "unit": "ns",
            "extra": "100 samples\n1178 iterations"
          },
          {
            "name": "OnNext",
            "value": 1.67159,
            "range": "± 0.00192771",
            "unit": "ns",
            "extra": "100 samples\n17818 iterations"
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
          "id": "538a41791e7bda36d418e81b0b9a03eaa714f789",
          "message": "Speedup std::function",
          "timestamp": "2022-02-06T00:38:44+03:00",
          "tree_id": "ed9f82ef546429513dd49fea149c167fd1be568a",
          "url": "https://github.com/victimsnino/ReactivePlusPlus/commit/538a41791e7bda36d418e81b0b9a03eaa714f789"
        },
        "date": 1644097191189,
        "tool": "catch2",
        "benches": [
          {
            "name": "Specific observable construction",
            "value": 60.6186,
            "range": "± 37.094",
            "unit": "ns",
            "extra": "100 samples\n610 iterations"
          },
          {
            "name": "Dynamic observable construction",
            "value": 80.1259,
            "range": "± 23.9791",
            "unit": "ns",
            "extra": "100 samples\n447 iterations"
          },
          {
            "name": "Specific observable subscribe",
            "value": 35.0967,
            "range": "± 1.96289",
            "unit": "ns",
            "extra": "100 samples\n868 iterations"
          },
          {
            "name": "Dynamic observable subscribe",
            "value": 38.6149,
            "range": "± 5.17364",
            "unit": "ns",
            "extra": "100 samples\n885 iterations"
          },
          {
            "name": "OnNext",
            "value": 1.8833,
            "range": "± 0.122601",
            "unit": "ns",
            "extra": "100 samples\n18131 iterations"
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
          "id": "2025ea96cb91c3eba5ed9d89c7e74a89189b3c56",
          "message": "Update Tests.yml",
          "timestamp": "2022-02-06T15:06:46+03:00",
          "tree_id": "38ca65c1414f7d89643a26b0105e6de78a28f392",
          "url": "https://github.com/victimsnino/ReactivePlusPlus/commit/2025ea96cb91c3eba5ed9d89c7e74a89189b3c56"
        },
        "date": 1644149271086,
        "tool": "catch2",
        "benches": [
          {
            "name": "Specific observable construction",
            "value": 60.4894,
            "range": "± 0.256969",
            "unit": "ns",
            "extra": "100 samples\n587 iterations"
          },
          {
            "name": "Dynamic observable construction",
            "value": 84.0265,
            "range": "± 1.2745",
            "unit": "ns",
            "extra": "100 samples\n427 iterations"
          },
          {
            "name": "Specific observable subscribe",
            "value": 41.0407,
            "range": "± 0.783754",
            "unit": "ns",
            "extra": "100 samples\n841 iterations"
          },
          {
            "name": "Dynamic observable subscribe",
            "value": 43.2537,
            "range": "± 0.87101",
            "unit": "ns",
            "extra": "100 samples\n821 iterations"
          },
          {
            "name": "OnNext",
            "value": 5.22865,
            "range": "± 0.0886571",
            "unit": "ns",
            "extra": "100 samples\n6802 iterations"
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
          "id": "51b0974744557e216a7b38df7665c38e5acc212f",
          "message": "Merge branch 'main' of github.com:victimsnino/ReactivePlusPlus",
          "timestamp": "2022-02-06T22:36:57+03:00",
          "tree_id": "1e14d8814cf69974878545a7cc5ef4a349ba0448",
          "url": "https://github.com/victimsnino/ReactivePlusPlus/commit/51b0974744557e216a7b38df7665c38e5acc212f"
        },
        "date": 1644176286534,
        "tool": "catch2",
        "benches": [
          {
            "name": "Specific observable construction",
            "value": 17.8781,
            "range": "± 5.68964",
            "unit": "ns",
            "extra": "100 samples\n1845 iterations"
          },
          {
            "name": "Dynamic observable construction",
            "value": 36.6485,
            "range": "± 12.2663",
            "unit": "ns",
            "extra": "100 samples\n919 iterations"
          },
          {
            "name": "Specific observable subscribe",
            "value": 36.3504,
            "range": "± 10.6908",
            "unit": "ns",
            "extra": "100 samples\n839 iterations"
          },
          {
            "name": "Dynamic observable subscribe",
            "value": 38.1821,
            "range": "± 9.72397",
            "unit": "ns",
            "extra": "100 samples\n873 iterations"
          },
          {
            "name": "OnNext",
            "value": 4.70861,
            "range": "± 1.63953",
            "unit": "ns",
            "extra": "100 samples\n6517 iterations"
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
          "id": "5876043aef50cebdc8ed16687a74c12befdb3513",
          "message": "Fix subscriber",
          "timestamp": "2022-02-06T23:51:24+03:00",
          "tree_id": "f67abed796d50f0b87331b57fe2ac9aed3c13351",
          "url": "https://github.com/victimsnino/ReactivePlusPlus/commit/5876043aef50cebdc8ed16687a74c12befdb3513"
        },
        "date": 1644180738949,
        "tool": "catch2",
        "benches": [
          {
            "name": "Specific observable construction",
            "value": 13.2371,
            "range": "± 0.397927",
            "unit": "ns",
            "extra": "100 samples\n1962 iterations"
          },
          {
            "name": "Dynamic observable construction",
            "value": 28.8,
            "range": "± 2.42607",
            "unit": "ns",
            "extra": "100 samples\n909 iterations"
          },
          {
            "name": "Specific observable subscribe",
            "value": 33.3886,
            "range": "± 2.59157",
            "unit": "ns",
            "extra": "100 samples\n788 iterations"
          },
          {
            "name": "Dynamic observable subscribe",
            "value": 33.3743,
            "range": "± 2.12938",
            "unit": "ns",
            "extra": "100 samples\n786 iterations"
          },
          {
            "name": "OnNext",
            "value": 3.54856,
            "range": "± 0.0583051",
            "unit": "ns",
            "extra": "100 samples\n7449 iterations"
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
          "id": "7540b25602aab337fe10100aff5fa52351db6edd",
          "message": "Compile fix",
          "timestamp": "2022-02-08T00:12:46+03:00",
          "tree_id": "812e847cc9199b726a1353ec0de11a2bc0e2575b",
          "url": "https://github.com/victimsnino/ReactivePlusPlus/commit/7540b25602aab337fe10100aff5fa52351db6edd"
        },
        "date": 1644268442313,
        "tool": "catch2",
        "benches": [
          {
            "name": "Specific observable construction",
            "value": 17.0441,
            "range": "± 3.49547",
            "unit": "ns",
            "extra": "100 samples\n1683 iterations"
          },
          {
            "name": "Dynamic observable construction",
            "value": 37.8421,
            "range": "± 7.5076",
            "unit": "ns",
            "extra": "100 samples\n790 iterations"
          },
          {
            "name": "Specific observable subscribe",
            "value": 21.7012,
            "range": "± 2.10681",
            "unit": "ns",
            "extra": "100 samples\n1204 iterations"
          },
          {
            "name": "Dynamic observable subscribe",
            "value": 22.4151,
            "range": "± 3.72367",
            "unit": "ns",
            "extra": "100 samples\n1258 iterations"
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
          "id": "f0a3a9dc389a8aaa5ec5fafed554e7f6316545ce",
          "message": "Enable perfomance on gcc",
          "timestamp": "2022-02-08T00:15:44+03:00",
          "tree_id": "9b611c2eb05f5fb08a6c0e1ce3597287721b8b61",
          "url": "https://github.com/victimsnino/ReactivePlusPlus/commit/f0a3a9dc389a8aaa5ec5fafed554e7f6316545ce"
        },
        "date": 1644268614731,
        "tool": "catch2",
        "benches": [
          {
            "name": "Specific observable construction",
            "value": 17.7035,
            "range": "± 0.280827",
            "unit": "ns",
            "extra": "100 samples\n1978 iterations"
          },
          {
            "name": "Dynamic observable construction",
            "value": 40.4229,
            "range": "± 3.22585",
            "unit": "ns",
            "extra": "100 samples\n888 iterations"
          },
          {
            "name": "Specific observable subscribe",
            "value": 24.8262,
            "range": "± 0.380862",
            "unit": "ns",
            "extra": "100 samples\n1441 iterations"
          },
          {
            "name": "Dynamic observable subscribe",
            "value": 25.2238,
            "range": "± 0.402359",
            "unit": "ns",
            "extra": "100 samples\n1405 iterations"
          }
        ]
      }
    ],
    "Catch2 Benchmark Linux GCC": [
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
          "id": "f0a3a9dc389a8aaa5ec5fafed554e7f6316545ce",
          "message": "Enable perfomance on gcc",
          "timestamp": "2022-02-08T00:15:44+03:00",
          "tree_id": "9b611c2eb05f5fb08a6c0e1ce3597287721b8b61",
          "url": "https://github.com/victimsnino/ReactivePlusPlus/commit/f0a3a9dc389a8aaa5ec5fafed554e7f6316545ce"
        },
        "date": 1644268626105,
        "tool": "catch2",
        "benches": [
          {
            "name": "Specific observable construction",
            "value": 21.1439,
            "range": "± 19.159",
            "unit": "ns",
            "extra": "100 samples\n1760 iterations"
          },
          {
            "name": "Dynamic observable construction",
            "value": 40.5643,
            "range": "± 4.14124",
            "unit": "ns",
            "extra": "100 samples\n741 iterations"
          },
          {
            "name": "Specific observable subscribe",
            "value": 24.3677,
            "range": "± 7.10942",
            "unit": "ns",
            "extra": "100 samples\n1393 iterations"
          },
          {
            "name": "Dynamic observable subscribe",
            "value": 23.1594,
            "range": "± 0.148767",
            "unit": "ns",
            "extra": "100 samples\n1305 iterations"
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
          "id": "17ca0316df5c19081827bb014e077617af5a72b0",
          "message": "Merge branch 'main' of github.com:victimsnino/ReactivePlusPlus",
          "timestamp": "2022-02-17T22:42:33+03:00",
          "tree_id": "516dd18d3a50afe4998153400203a30c8017d41a",
          "url": "https://github.com/victimsnino/ReactivePlusPlus/commit/17ca0316df5c19081827bb014e077617af5a72b0"
        },
        "date": 1645127006914,
        "tool": "catch2",
        "benches": [
          {
            "name": "Specific observable construction",
            "value": 12.9229,
            "range": "± 0.942577",
            "unit": "ns",
            "extra": "100 samples\n2045 iterations"
          },
          {
            "name": "Dynamic observable construction",
            "value": 30.1605,
            "range": "± 1.71694",
            "unit": "ns",
            "extra": "100 samples\n862 iterations"
          },
          {
            "name": "Specific observable subscribe",
            "value": 16.0346,
            "range": "± 1.45987",
            "unit": "ns",
            "extra": "100 samples\n1656 iterations"
          },
          {
            "name": "Dynamic observable subscribe",
            "value": 16.0531,
            "range": "± 0.768246",
            "unit": "ns",
            "extra": "100 samples\n1587 iterations"
          }
        ]
      }
    ]
  }
}