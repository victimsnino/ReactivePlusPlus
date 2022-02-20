window.BENCHMARK_DATA = {
  "lastUpdate": 1645347452352,
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
        "date": 1645127014007,
        "tool": "catch2",
        "benches": [
          {
            "name": "Specific observable construction",
            "value": 14.8026,
            "range": "± 0.276302",
            "unit": "ns",
            "extra": "100 samples\n1995 iterations"
          },
          {
            "name": "Dynamic observable construction",
            "value": 33.8888,
            "range": "± 0.686607",
            "unit": "ns",
            "extra": "100 samples\n880 iterations"
          },
          {
            "name": "Specific observable subscribe",
            "value": 20.6718,
            "range": "± 0.353553",
            "unit": "ns",
            "extra": "100 samples\n1453 iterations"
          },
          {
            "name": "Dynamic observable subscribe",
            "value": 21.0371,
            "range": "± 0.33743",
            "unit": "ns",
            "extra": "100 samples\n1435 iterations"
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
          "id": "581bcefb77494b3cb741a20fe399ee4a50ff6223",
          "message": "Try to enable perfomance tests on win (#13)",
          "timestamp": "2022-02-17T23:17:47+03:00",
          "tree_id": "48eded44490e83647eda8d81687af91a88a88051",
          "url": "https://github.com/victimsnino/ReactivePlusPlus/commit/581bcefb77494b3cb741a20fe399ee4a50ff6223"
        },
        "date": 1645129134581,
        "tool": "catch2",
        "benches": [
          {
            "name": "Specific observable construction",
            "value": 15.6323,
            "range": "± 4.9453",
            "unit": "ns",
            "extra": "100 samples\n1868 iterations"
          },
          {
            "name": "Dynamic observable construction",
            "value": 38.131,
            "range": "± 16.0499",
            "unit": "ns",
            "extra": "100 samples\n818 iterations"
          },
          {
            "name": "Specific observable subscribe",
            "value": 39.187,
            "range": "± 15.7861",
            "unit": "ns",
            "extra": "100 samples\n771 iterations"
          },
          {
            "name": "Dynamic observable subscribe",
            "value": 36.0776,
            "range": "± 9.20791",
            "unit": "ns",
            "extra": "100 samples\n762 iterations"
          },
          {
            "name": "OnNext",
            "value": 6.96357,
            "range": "± 17.8374",
            "unit": "ns",
            "extra": "100 samples\n6528 iterations"
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
          "id": "85b7c64d8dbcc229c1d8734cc938a1cebf35b20c",
          "message": "Compile fix",
          "timestamp": "2022-02-19T22:31:12+03:00",
          "tree_id": "c163565131c13ca17cc33af222a1ba05fbed6913",
          "url": "https://github.com/victimsnino/ReactivePlusPlus/commit/85b7c64d8dbcc229c1d8734cc938a1cebf35b20c"
        },
        "date": 1645299138326,
        "tool": "catch2",
        "benches": [
          {
            "name": "Specific observable construction",
            "value": 21.5248,
            "range": "± 3.13823",
            "unit": "ns",
            "extra": "100 samples\n1317 iterations"
          },
          {
            "name": "Dynamic observable construction",
            "value": 36.4915,
            "range": "± 6.5506",
            "unit": "ns",
            "extra": "100 samples\n857 iterations"
          },
          {
            "name": "Specific observable subscribe",
            "value": 26.2012,
            "range": "± 3.28604",
            "unit": "ns",
            "extra": "100 samples\n1064 iterations"
          },
          {
            "name": "Dynamic observable subscribe",
            "value": 24.7052,
            "range": "± 4.17409",
            "unit": "ns",
            "extra": "100 samples\n1207 iterations"
          },
          {
            "name": "OnNext",
            "value": 1.62313,
            "range": "± 0.0623186",
            "unit": "ns",
            "extra": "100 samples\n18011 iterations"
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
          "id": "5cd298539b8da25f7cc896011dbf2e1898b0a24a",
          "message": "Revert subscriber split",
          "timestamp": "2022-02-17T23:07:02+03:00",
          "tree_id": "5d9097b1dca3fa9322514c16d29a4afdd4657297",
          "url": "https://github.com/victimsnino/ReactivePlusPlus/commit/5cd298539b8da25f7cc896011dbf2e1898b0a24a"
        },
        "date": 1645128485858,
        "tool": "catch2",
        "benches": [
          {
            "name": "Specific observable construction",
            "value": 13.159,
            "range": "± 0.284128",
            "unit": "ns",
            "extra": "100 samples\n2209 iterations"
          },
          {
            "name": "Dynamic observable construction",
            "value": 34.8511,
            "range": "± 0.711249",
            "unit": "ns",
            "extra": "100 samples\n845 iterations"
          },
          {
            "name": "Specific observable subscribe",
            "value": 35.1827,
            "range": "± 0.208796",
            "unit": "ns",
            "extra": "100 samples\n833 iterations"
          },
          {
            "name": "Dynamic observable subscribe",
            "value": 36.8586,
            "range": "± 0.84793",
            "unit": "ns",
            "extra": "100 samples\n812 iterations"
          },
          {
            "name": "OnNext",
            "value": 4.68722,
            "range": "± 0.067884",
            "unit": "ns",
            "extra": "100 samples\n6301 iterations"
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
          "id": "581bcefb77494b3cb741a20fe399ee4a50ff6223",
          "message": "Try to enable perfomance tests on win (#13)",
          "timestamp": "2022-02-17T23:17:47+03:00",
          "tree_id": "48eded44490e83647eda8d81687af91a88a88051",
          "url": "https://github.com/victimsnino/ReactivePlusPlus/commit/581bcefb77494b3cb741a20fe399ee4a50ff6223"
        },
        "date": 1645129138781,
        "tool": "catch2",
        "benches": [
          {
            "name": "Specific observable construction",
            "value": 17.8764,
            "range": "± 3.59287",
            "unit": "ns",
            "extra": "100 samples\n1948 iterations"
          },
          {
            "name": "Dynamic observable construction",
            "value": 37.9879,
            "range": "± 6.25974",
            "unit": "ns",
            "extra": "100 samples\n809 iterations"
          },
          {
            "name": "Specific observable subscribe",
            "value": 38.0606,
            "range": "± 8.36943",
            "unit": "ns",
            "extra": "100 samples\n778 iterations"
          },
          {
            "name": "Dynamic observable subscribe",
            "value": 38.1904,
            "range": "± 7.91337",
            "unit": "ns",
            "extra": "100 samples\n757 iterations"
          },
          {
            "name": "OnNext",
            "value": 5.39681,
            "range": "± 1.22526",
            "unit": "ns",
            "extra": "100 samples\n5852 iterations"
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
          "id": "85b7c64d8dbcc229c1d8734cc938a1cebf35b20c",
          "message": "Compile fix",
          "timestamp": "2022-02-19T22:31:12+03:00",
          "tree_id": "c163565131c13ca17cc33af222a1ba05fbed6913",
          "url": "https://github.com/victimsnino/ReactivePlusPlus/commit/85b7c64d8dbcc229c1d8734cc938a1cebf35b20c"
        },
        "date": 1645299143687,
        "tool": "catch2",
        "benches": [
          {
            "name": "Specific observable construction",
            "value": 23.5081,
            "range": "± 6.02122",
            "unit": "ns",
            "extra": "100 samples\n1262 iterations"
          },
          {
            "name": "Dynamic observable construction",
            "value": 47.7178,
            "range": "± 9.55937",
            "unit": "ns",
            "extra": "100 samples\n662 iterations"
          },
          {
            "name": "Specific observable subscribe",
            "value": 31.6437,
            "range": "± 11.307",
            "unit": "ns",
            "extra": "100 samples\n1063 iterations"
          },
          {
            "name": "Dynamic observable subscribe",
            "value": 27.6474,
            "range": "± 4.06571",
            "unit": "ns",
            "extra": "100 samples\n1077 iterations"
          },
          {
            "name": "OnNext",
            "value": 2.17491,
            "range": "± 0.49719",
            "unit": "ns",
            "extra": "100 samples\n13597 iterations"
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
          "id": "912562b53fcab2938cb43d51869ec62bd7e9b774",
          "message": "Split observers (#14)\n\n* Extract different types of observers\r\n\r\n* Modify observables\r\n\r\n* Try compile fix\r\n\r\n* One more\r\n\r\n* Fix optimize away\r\n\r\n* Try to fix\r\n\r\n* Compile\r\n\r\n* Temporarly disable logging\r\n\r\n* Try to avoid optimize\r\n\r\n* Revert \"Temporarly disable logging\"\r\n\r\nThis reverts commit 623006ec64e10596aee00c421d07a28f32b18710.",
          "timestamp": "2022-02-20T11:56:34+03:00",
          "tree_id": "29df1515f5da68e0f4f13613d49ebb0e75f48def",
          "url": "https://github.com/victimsnino/ReactivePlusPlus/commit/912562b53fcab2938cb43d51869ec62bd7e9b774"
        },
        "date": 1645347451569,
        "tool": "catch2",
        "benches": [
          {
            "name": "Specific observable construction",
            "value": 0.670137,
            "range": "± 0.0105419",
            "unit": "ns",
            "extra": "100 samples\n44008 iterations"
          },
          {
            "name": "Dynamic observable construction",
            "value": 19.1345,
            "range": "± 0.127746",
            "unit": "ns",
            "extra": "100 samples\n1542 iterations"
          },
          {
            "name": "Specific observable subscribe",
            "value": 20.3412,
            "range": "± 0.34942",
            "unit": "ns",
            "extra": "100 samples\n1405 iterations"
          },
          {
            "name": "Dynamic observable subscribe",
            "value": 45.1321,
            "range": "± 0.996136",
            "unit": "ns",
            "extra": "100 samples\n654 iterations"
          },
          {
            "name": "OnNext",
            "value": 0.334968,
            "range": "± 0.00440437",
            "unit": "ns",
            "extra": "100 samples\n88129 iterations"
          }
        ]
      }
    ],
    "Catch2 Benchmark Windows MSVC": [
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
          "id": "581bcefb77494b3cb741a20fe399ee4a50ff6223",
          "message": "Try to enable perfomance tests on win (#13)",
          "timestamp": "2022-02-17T23:17:47+03:00",
          "tree_id": "48eded44490e83647eda8d81687af91a88a88051",
          "url": "https://github.com/victimsnino/ReactivePlusPlus/commit/581bcefb77494b3cb741a20fe399ee4a50ff6223"
        },
        "date": 1645129376836,
        "tool": "catch2",
        "benches": [
          {
            "name": "Specific observable construction",
            "value": 22.8036,
            "range": "± 0.368645",
            "unit": "ns",
            "extra": "100 samples\n2251 iterations"
          },
          {
            "name": "Dynamic observable construction",
            "value": 92.7792,
            "range": "± 1.84436",
            "unit": "ns",
            "extra": "100 samples\n548 iterations"
          },
          {
            "name": "Specific observable subscribe",
            "value": 97.2173,
            "range": "± 1.56758",
            "unit": "ns",
            "extra": "100 samples\n474 iterations"
          },
          {
            "name": "Dynamic observable subscribe",
            "value": 97.5458,
            "range": "± 1.55096",
            "unit": "ns",
            "extra": "100 samples\n535 iterations"
          },
          {
            "name": "OnNext",
            "value": 3.55168,
            "range": "± 0.039725",
            "unit": "ns",
            "extra": "100 samples\n14619 iterations"
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
          "id": "85b7c64d8dbcc229c1d8734cc938a1cebf35b20c",
          "message": "Compile fix",
          "timestamp": "2022-02-19T22:31:12+03:00",
          "tree_id": "c163565131c13ca17cc33af222a1ba05fbed6913",
          "url": "https://github.com/victimsnino/ReactivePlusPlus/commit/85b7c64d8dbcc229c1d8734cc938a1cebf35b20c"
        },
        "date": 1645299195860,
        "tool": "catch2",
        "benches": [
          {
            "name": "Specific observable construction",
            "value": 98.7369,
            "range": "± 2.47981",
            "unit": "ns",
            "extra": "100 samples\n593 iterations"
          },
          {
            "name": "Dynamic observable construction",
            "value": 178.832,
            "range": "± 2.7674",
            "unit": "ns",
            "extra": "100 samples\n328 iterations"
          },
          {
            "name": "Specific observable subscribe",
            "value": 115.216,
            "range": "± 2.51773",
            "unit": "ns",
            "extra": "100 samples\n509 iterations"
          },
          {
            "name": "Dynamic observable subscribe",
            "value": 116.261,
            "range": "± 8.50495",
            "unit": "ns",
            "extra": "100 samples\n514 iterations"
          },
          {
            "name": "OnNext",
            "value": 1.68137,
            "range": "± 0.0386159",
            "unit": "ns",
            "extra": "100 samples\n34626 iterations"
          }
        ]
      }
    ]
  }
}