window.BENCHMARK_DATA = {
  "lastUpdate": 1646424059418,
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
        "date": 1645347467749,
        "tool": "catch2",
        "benches": [
          {
            "name": "Specific observable construction",
            "value": 0.837013,
            "range": "± 0.0148003",
            "unit": "ns",
            "extra": "100 samples\n26826 iterations"
          },
          {
            "name": "Dynamic observable construction",
            "value": 16.7472,
            "range": "± 3.20064",
            "unit": "ns",
            "extra": "100 samples\n1469 iterations"
          },
          {
            "name": "Specific observable subscribe",
            "value": 16.8656,
            "range": "± 1.11737",
            "unit": "ns",
            "extra": "100 samples\n1580 iterations"
          },
          {
            "name": "Dynamic observable subscribe",
            "value": 36.3905,
            "range": "± 0.736538",
            "unit": "ns",
            "extra": "100 samples\n665 iterations"
          },
          {
            "name": "OnNext",
            "value": 0.6605,
            "range": "± 0.216189",
            "unit": "ns",
            "extra": "100 samples\n41359 iterations"
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
          "id": "53f49c43fd812730fdfd6645f518dc66c0cb5b75",
          "message": "Simplify traits a bit",
          "timestamp": "2022-02-27T21:42:14+03:00",
          "tree_id": "e824b0e7ad1e260569d8d60d4cf539aa54e6c1f4",
          "url": "https://github.com/victimsnino/ReactivePlusPlus/commit/53f49c43fd812730fdfd6645f518dc66c0cb5b75"
        },
        "date": 1645987403347,
        "tool": "catch2",
        "benches": [
          {
            "name": "Specific observable construction",
            "value": 1.06706,
            "range": "± 0.384322",
            "unit": "ns",
            "extra": "100 samples\n29330 iterations"
          },
          {
            "name": "Dynamic observable construction",
            "value": 14.9792,
            "range": "± 1.12307",
            "unit": "ns",
            "extra": "100 samples\n1542 iterations"
          },
          {
            "name": "Specific observable subscribe",
            "value": 24.1308,
            "range": "± 29.8199",
            "unit": "ns",
            "extra": "100 samples\n1566 iterations"
          },
          {
            "name": "Dynamic observable subscribe",
            "value": 37.5804,
            "range": "± 7.70425",
            "unit": "ns",
            "extra": "100 samples\n751 iterations"
          },
          {
            "name": "OnNext",
            "value": 0.612227,
            "range": "± 0.22858",
            "unit": "ns",
            "extra": "100 samples\n41981 iterations"
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
          "id": "bee9386d4edc3ee0afad0e1707305d2b9408dd62",
          "message": "Simplify type-traits",
          "timestamp": "2022-02-27T22:03:35+03:00",
          "tree_id": "74f3b796ba5e9e508445ab7ea91e0d2d09c851fb",
          "url": "https://github.com/victimsnino/ReactivePlusPlus/commit/bee9386d4edc3ee0afad0e1707305d2b9408dd62"
        },
        "date": 1645988671725,
        "tool": "catch2",
        "benches": [
          {
            "name": "Specific observable construction",
            "value": 1.00488,
            "range": "± 0.0140625",
            "unit": "ns",
            "extra": "100 samples\n29549 iterations"
          },
          {
            "name": "Dynamic observable construction",
            "value": 19.5964,
            "range": "± 0.455691",
            "unit": "ns",
            "extra": "100 samples\n1506 iterations"
          },
          {
            "name": "Specific observable subscribe",
            "value": 19.4171,
            "range": "± 0.590204",
            "unit": "ns",
            "extra": "100 samples\n1511 iterations"
          },
          {
            "name": "Dynamic observable subscribe",
            "value": 42.8015,
            "range": "± 0.671313",
            "unit": "ns",
            "extra": "100 samples\n679 iterations"
          },
          {
            "name": "OnNext",
            "value": 0.66883,
            "range": "± 0.00123929",
            "unit": "ns",
            "extra": "100 samples\n44324 iterations"
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
          "id": "f06e8f6c4f607a482466adf37ad4c6f25762f486",
          "message": "Small cleanup",
          "timestamp": "2022-02-27T22:16:44+03:00",
          "tree_id": "e801d2cb5f949f26ea647a346a2607b32cd84f25",
          "url": "https://github.com/victimsnino/ReactivePlusPlus/commit/f06e8f6c4f607a482466adf37ad4c6f25762f486"
        },
        "date": 1645989460770,
        "tool": "catch2",
        "benches": [
          {
            "name": "Specific observable construction",
            "value": 1.00472,
            "range": "± 0.014648",
            "unit": "ns",
            "extra": "100 samples\n29732 iterations"
          },
          {
            "name": "Dynamic observable construction",
            "value": 19.4284,
            "range": "± 0.173235",
            "unit": "ns",
            "extra": "100 samples\n1516 iterations"
          },
          {
            "name": "Specific observable subscribe",
            "value": 19.4283,
            "range": "± 0.469188",
            "unit": "ns",
            "extra": "100 samples\n1514 iterations"
          },
          {
            "name": "Dynamic observable subscribe",
            "value": 43.4388,
            "range": "± 0.639004",
            "unit": "ns",
            "extra": "100 samples\n696 iterations"
          },
          {
            "name": "OnNext",
            "value": 0.669239,
            "range": "± 0.00322575",
            "unit": "ns",
            "extra": "100 samples\n44593 iterations"
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
          "id": "1d0d8cf57cf72cc4170d013c4a2732e77ded8064",
          "message": "small fix",
          "timestamp": "2022-02-27T22:35:29+03:00",
          "tree_id": "9af0e3214116d7228360d2993f7ba12d07febaf2",
          "url": "https://github.com/victimsnino/ReactivePlusPlus/commit/1d0d8cf57cf72cc4170d013c4a2732e77ded8064"
        },
        "date": 1645990587690,
        "tool": "catch2",
        "benches": [
          {
            "name": "Specific observable construction",
            "value": 1.00557,
            "range": "± 0.0142648",
            "unit": "ns",
            "extra": "100 samples\n29840 iterations"
          },
          {
            "name": "Dynamic observable construction",
            "value": 19.4224,
            "range": "± 0.284317",
            "unit": "ns",
            "extra": "100 samples\n1530 iterations"
          },
          {
            "name": "Specific observable subscribe",
            "value": 19.5742,
            "range": "± 0.412835",
            "unit": "ns",
            "extra": "100 samples\n1559 iterations"
          },
          {
            "name": "Dynamic observable subscribe",
            "value": 43.4149,
            "range": "± 0.936135",
            "unit": "ns",
            "extra": "100 samples\n685 iterations"
          },
          {
            "name": "OnNext",
            "value": 0.669576,
            "range": "± 0.00159369",
            "unit": "ns",
            "extra": "100 samples\n44760 iterations"
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
          "id": "0bd4287bfb7eb80cbcc5c568efc63d1c257259b2",
          "message": "Split subscriber types (#15)",
          "timestamp": "2022-03-02T23:43:31+03:00",
          "tree_id": "b34bce5f5def265d5ea6ad1179dbf62aac5b4e29",
          "url": "https://github.com/victimsnino/ReactivePlusPlus/commit/0bd4287bfb7eb80cbcc5c568efc63d1c257259b2"
        },
        "date": 1646254248080,
        "tool": "catch2",
        "benches": [
          {
            "name": "Specific observable construction",
            "value": 1.11347,
            "range": "± 1.20138",
            "unit": "ns",
            "extra": "100 samples\n28705 iterations"
          },
          {
            "name": "Dynamic observable construction",
            "value": 21.1597,
            "range": "± 4.11039",
            "unit": "ns",
            "extra": "100 samples\n1429 iterations"
          },
          {
            "name": "Specific observable subscribe",
            "value": 26.6251,
            "range": "± 7.14693",
            "unit": "ns",
            "extra": "100 samples\n1317 iterations"
          },
          {
            "name": "Dynamic observable subscribe",
            "value": 48.4172,
            "range": "± 18.2872",
            "unit": "ns",
            "extra": "100 samples\n650 iterations"
          },
          {
            "name": "OnNext",
            "value": 0.743576,
            "range": "± 0.263599",
            "unit": "ns",
            "extra": "100 samples\n45077 iterations"
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
          "id": "6c2ce1eced8dc5fc7514eec78c73007f43425010",
          "message": "Try to use cache (#16)",
          "timestamp": "2022-03-04T00:33:56+03:00",
          "tree_id": "d39db99766b95bdacd1f96d25f147d426d08ab87",
          "url": "https://github.com/victimsnino/ReactivePlusPlus/commit/6c2ce1eced8dc5fc7514eec78c73007f43425010"
        },
        "date": 1646343323706,
        "tool": "catch2",
        "benches": [
          {
            "name": "Specific observable construction",
            "value": 0.848916,
            "range": "± 0.060652",
            "unit": "ns",
            "extra": "100 samples\n25740 iterations"
          },
          {
            "name": "Dynamic observable construction",
            "value": 19.6205,
            "range": "± 6.09707",
            "unit": "ns",
            "extra": "100 samples\n1678 iterations"
          },
          {
            "name": "Specific observable subscribe",
            "value": 21.7062,
            "range": "± 4.1625",
            "unit": "ns",
            "extra": "100 samples\n1385 iterations"
          },
          {
            "name": "Dynamic observable subscribe",
            "value": 60.7921,
            "range": "± 28.3902",
            "unit": "ns",
            "extra": "100 samples\n634 iterations"
          },
          {
            "name": "OnNext",
            "value": 0.763708,
            "range": "± 0.20918",
            "unit": "ns",
            "extra": "100 samples\n46848 iterations"
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
          "id": "929869a759c5094b75f275d4b6c1dfc6809cfd19",
          "message": "Small reorg",
          "timestamp": "2022-03-04T22:58:27+03:00",
          "tree_id": "d0f209227cb6318c8d27b6268cba8f8490467809",
          "url": "https://github.com/victimsnino/ReactivePlusPlus/commit/929869a759c5094b75f275d4b6c1dfc6809cfd19"
        },
        "date": 1646423950632,
        "tool": "catch2",
        "benches": [
          {
            "name": "Specific observable construction",
            "value": 1.0033,
            "range": "± 0.00163987",
            "unit": "ns",
            "extra": "100 samples\n29714 iterations"
          },
          {
            "name": "Dynamic observable construction",
            "value": 19.3019,
            "range": "± 0.510646",
            "unit": "ns",
            "extra": "100 samples\n1551 iterations"
          },
          {
            "name": "Specific observable subscribe",
            "value": 24.6252,
            "range": "± 0.483639",
            "unit": "ns",
            "extra": "100 samples\n1235 iterations"
          },
          {
            "name": "Dynamic observable subscribe",
            "value": 47.438,
            "range": "± 1.15446",
            "unit": "ns",
            "extra": "100 samples\n626 iterations"
          },
          {
            "name": "OnNext",
            "value": 0.669683,
            "range": "± 0.0100104",
            "unit": "ns",
            "extra": "100 samples\n44590 iterations"
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
          "id": "53f49c43fd812730fdfd6645f518dc66c0cb5b75",
          "message": "Simplify traits a bit",
          "timestamp": "2022-02-27T21:42:14+03:00",
          "tree_id": "e824b0e7ad1e260569d8d60d4cf539aa54e6c1f4",
          "url": "https://github.com/victimsnino/ReactivePlusPlus/commit/53f49c43fd812730fdfd6645f518dc66c0cb5b75"
        },
        "date": 1645987393374,
        "tool": "catch2",
        "benches": [
          {
            "name": "Specific observable construction",
            "value": 0.67016,
            "range": "± 0.0103557",
            "unit": "ns",
            "extra": "100 samples\n43850 iterations"
          },
          {
            "name": "Dynamic observable construction",
            "value": 19.2129,
            "range": "± 0.340673",
            "unit": "ns",
            "extra": "100 samples\n1543 iterations"
          },
          {
            "name": "Specific observable subscribe",
            "value": 20.5049,
            "range": "± 0.346397",
            "unit": "ns",
            "extra": "100 samples\n1433 iterations"
          },
          {
            "name": "Dynamic observable subscribe",
            "value": 46.0086,
            "range": "± 0.824523",
            "unit": "ns",
            "extra": "100 samples\n638 iterations"
          },
          {
            "name": "OnNext",
            "value": 0.335992,
            "range": "± 0.0138993",
            "unit": "ns",
            "extra": "100 samples\n87709 iterations"
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
          "id": "bee9386d4edc3ee0afad0e1707305d2b9408dd62",
          "message": "Simplify type-traits",
          "timestamp": "2022-02-27T22:03:35+03:00",
          "tree_id": "74f3b796ba5e9e508445ab7ea91e0d2d09c851fb",
          "url": "https://github.com/victimsnino/ReactivePlusPlus/commit/bee9386d4edc3ee0afad0e1707305d2b9408dd62"
        },
        "date": 1645988675648,
        "tool": "catch2",
        "benches": [
          {
            "name": "Specific observable construction",
            "value": 0.670535,
            "range": "± 0.0122147",
            "unit": "ns",
            "extra": "100 samples\n44037 iterations"
          },
          {
            "name": "Dynamic observable construction",
            "value": 18.873,
            "range": "± 0.310082",
            "unit": "ns",
            "extra": "100 samples\n1548 iterations"
          },
          {
            "name": "Specific observable subscribe",
            "value": 20.9563,
            "range": "± 1.13574",
            "unit": "ns",
            "extra": "100 samples\n1435 iterations"
          },
          {
            "name": "Dynamic observable subscribe",
            "value": 46.1351,
            "range": "± 0.657479",
            "unit": "ns",
            "extra": "100 samples\n639 iterations"
          },
          {
            "name": "OnNext",
            "value": 0.335089,
            "range": "± 0.00544215",
            "unit": "ns",
            "extra": "100 samples\n88100 iterations"
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
          "id": "f06e8f6c4f607a482466adf37ad4c6f25762f486",
          "message": "Small cleanup",
          "timestamp": "2022-02-27T22:16:44+03:00",
          "tree_id": "e801d2cb5f949f26ea647a346a2607b32cd84f25",
          "url": "https://github.com/victimsnino/ReactivePlusPlus/commit/f06e8f6c4f607a482466adf37ad4c6f25762f486"
        },
        "date": 1645989469157,
        "tool": "catch2",
        "benches": [
          {
            "name": "Specific observable construction",
            "value": 0.790349,
            "range": "± 0.0683102",
            "unit": "ns",
            "extra": "100 samples\n43251 iterations"
          },
          {
            "name": "Dynamic observable construction",
            "value": 22.8286,
            "range": "± 0.414058",
            "unit": "ns",
            "extra": "100 samples\n1520 iterations"
          },
          {
            "name": "Specific observable subscribe",
            "value": 25.1765,
            "range": "± 1.88194",
            "unit": "ns",
            "extra": "100 samples\n1372 iterations"
          },
          {
            "name": "Dynamic observable subscribe",
            "value": 56.7454,
            "range": "± 5.33496",
            "unit": "ns",
            "extra": "100 samples\n620 iterations"
          },
          {
            "name": "OnNext",
            "value": 0.402157,
            "range": "± 0.0054128",
            "unit": "ns",
            "extra": "100 samples\n82006 iterations"
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
          "id": "1d0d8cf57cf72cc4170d013c4a2732e77ded8064",
          "message": "small fix",
          "timestamp": "2022-02-27T22:35:29+03:00",
          "tree_id": "9af0e3214116d7228360d2993f7ba12d07febaf2",
          "url": "https://github.com/victimsnino/ReactivePlusPlus/commit/1d0d8cf57cf72cc4170d013c4a2732e77ded8064"
        },
        "date": 1645990590125,
        "tool": "catch2",
        "benches": [
          {
            "name": "Specific observable construction",
            "value": 0.671182,
            "range": "± 0.0136625",
            "unit": "ns",
            "extra": "100 samples\n44112 iterations"
          },
          {
            "name": "Dynamic observable construction",
            "value": 19.6324,
            "range": "± 0.882126",
            "unit": "ns",
            "extra": "100 samples\n1512 iterations"
          },
          {
            "name": "Specific observable subscribe",
            "value": 20.0505,
            "range": "± 0.546585",
            "unit": "ns",
            "extra": "100 samples\n1469 iterations"
          },
          {
            "name": "Dynamic observable subscribe",
            "value": 45.7827,
            "range": "± 0.31613",
            "unit": "ns",
            "extra": "100 samples\n648 iterations"
          },
          {
            "name": "OnNext",
            "value": 0.335097,
            "range": "± 0.000831581",
            "unit": "ns",
            "extra": "100 samples\n88157 iterations"
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
          "id": "402c711797af13ea4899c448b4573012e37430f8",
          "message": "Minor changes",
          "timestamp": "2022-03-02T22:06:16+03:00",
          "tree_id": "2fbb7a0b5c50783bbbbf3018c64feff723225858",
          "url": "https://github.com/victimsnino/ReactivePlusPlus/commit/402c711797af13ea4899c448b4573012e37430f8"
        },
        "date": 1646251614897,
        "tool": "catch2",
        "benches": [
          {
            "name": "Specific observable construction",
            "value": 0.671715,
            "range": "± 0.0235454",
            "unit": "ns",
            "extra": "100 samples\n44042 iterations"
          },
          {
            "name": "Dynamic observable construction",
            "value": 20.317,
            "range": "± 0.266896",
            "unit": "ns",
            "extra": "100 samples\n1491 iterations"
          },
          {
            "name": "Specific observable subscribe",
            "value": 20.8737,
            "range": "± 0.668593",
            "unit": "ns",
            "extra": "100 samples\n1443 iterations"
          },
          {
            "name": "Dynamic observable subscribe",
            "value": 45.888,
            "range": "± 0.688668",
            "unit": "ns",
            "extra": "100 samples\n646 iterations"
          },
          {
            "name": "OnNext",
            "value": 0.334797,
            "range": "± 0.00204709",
            "unit": "ns",
            "extra": "100 samples\n88112 iterations"
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
          "id": "0bd4287bfb7eb80cbcc5c568efc63d1c257259b2",
          "message": "Split subscriber types (#15)",
          "timestamp": "2022-03-02T23:43:31+03:00",
          "tree_id": "b34bce5f5def265d5ea6ad1179dbf62aac5b4e29",
          "url": "https://github.com/victimsnino/ReactivePlusPlus/commit/0bd4287bfb7eb80cbcc5c568efc63d1c257259b2"
        },
        "date": 1646254261203,
        "tool": "catch2",
        "benches": [
          {
            "name": "Specific observable construction",
            "value": 0.608216,
            "range": "± 0.00176198",
            "unit": "ns",
            "extra": "100 samples\n41306 iterations"
          },
          {
            "name": "Dynamic observable construction",
            "value": 22.9313,
            "range": "± 10.0417",
            "unit": "ns",
            "extra": "100 samples\n1543 iterations"
          },
          {
            "name": "Specific observable subscribe",
            "value": 21.547,
            "range": "± 8.93109",
            "unit": "ns",
            "extra": "100 samples\n1362 iterations"
          },
          {
            "name": "Dynamic observable subscribe",
            "value": 48.288,
            "range": "± 4.72538",
            "unit": "ns",
            "extra": "100 samples\n552 iterations"
          },
          {
            "name": "OnNext",
            "value": 0.320382,
            "range": "± 0.0608541",
            "unit": "ns",
            "extra": "100 samples\n81200 iterations"
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
          "id": "6c2ce1eced8dc5fc7514eec78c73007f43425010",
          "message": "Try to use cache (#16)",
          "timestamp": "2022-03-04T00:33:56+03:00",
          "tree_id": "d39db99766b95bdacd1f96d25f147d426d08ab87",
          "url": "https://github.com/victimsnino/ReactivePlusPlus/commit/6c2ce1eced8dc5fc7514eec78c73007f43425010"
        },
        "date": 1646343311711,
        "tool": "catch2",
        "benches": [
          {
            "name": "Specific observable construction",
            "value": 1.42147,
            "range": "± 0.228709",
            "unit": "ns",
            "extra": "100 samples\n21314 iterations"
          },
          {
            "name": "Dynamic observable construction",
            "value": 16.4769,
            "range": "± 0.24491",
            "unit": "ns",
            "extra": "100 samples\n1480 iterations"
          },
          {
            "name": "Specific observable subscribe",
            "value": 22.4134,
            "range": "± 3.26709",
            "unit": "ns",
            "extra": "100 samples\n1416 iterations"
          },
          {
            "name": "Dynamic observable subscribe",
            "value": 45.4404,
            "range": "± 5.01094",
            "unit": "ns",
            "extra": "100 samples\n547 iterations"
          },
          {
            "name": "OnNext",
            "value": 0.307027,
            "range": "± 0.0347183",
            "unit": "ns",
            "extra": "100 samples\n94635 iterations"
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
          "id": "929869a759c5094b75f275d4b6c1dfc6809cfd19",
          "message": "Small reorg",
          "timestamp": "2022-03-04T22:58:27+03:00",
          "tree_id": "d0f209227cb6318c8d27b6268cba8f8490467809",
          "url": "https://github.com/victimsnino/ReactivePlusPlus/commit/929869a759c5094b75f275d4b6c1dfc6809cfd19"
        },
        "date": 1646423953310,
        "tool": "catch2",
        "benches": [
          {
            "name": "Specific observable construction",
            "value": 1.58979,
            "range": "± 0.146518",
            "unit": "ns",
            "extra": "100 samples\n22450 iterations"
          },
          {
            "name": "Dynamic observable construction",
            "value": 23.2424,
            "range": "± 2.17047",
            "unit": "ns",
            "extra": "100 samples\n1530 iterations"
          },
          {
            "name": "Specific observable subscribe",
            "value": 25.2868,
            "range": "± 2.32442",
            "unit": "ns",
            "extra": "100 samples\n1396 iterations"
          },
          {
            "name": "Dynamic observable subscribe",
            "value": 60.5997,
            "range": "± 1.10364",
            "unit": "ns",
            "extra": "100 samples\n596 iterations"
          },
          {
            "name": "OnNext",
            "value": 0.403889,
            "range": "± 0.0201559",
            "unit": "ns",
            "extra": "100 samples\n89382 iterations"
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
          "id": "1b82f2d5f273a50c9fc2401ba590c20f6ed2ac12",
          "message": "One more",
          "timestamp": "2022-03-04T22:59:05+03:00",
          "tree_id": "f544526225ea543dfa408f4fc74429a077cf83c3",
          "url": "https://github.com/victimsnino/ReactivePlusPlus/commit/1b82f2d5f273a50c9fc2401ba590c20f6ed2ac12"
        },
        "date": 1646424058941,
        "tool": "catch2",
        "benches": [
          {
            "name": "Specific observable construction",
            "value": 1.6069,
            "range": "± 0.0201968",
            "unit": "ns",
            "extra": "100 samples\n21843 iterations"
          },
          {
            "name": "Dynamic observable construction",
            "value": 23.7221,
            "range": "± 0.528658",
            "unit": "ns",
            "extra": "100 samples\n1490 iterations"
          },
          {
            "name": "Specific observable subscribe",
            "value": 25.7413,
            "range": "± 0.445909",
            "unit": "ns",
            "extra": "100 samples\n1353 iterations"
          },
          {
            "name": "Dynamic observable subscribe",
            "value": 61.9257,
            "range": "± 0.842218",
            "unit": "ns",
            "extra": "100 samples\n580 iterations"
          },
          {
            "name": "OnNext",
            "value": 0.402169,
            "range": "± 0.0061301",
            "unit": "ns",
            "extra": "100 samples\n87417 iterations"
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
        "date": 1645347696819,
        "tool": "catch2",
        "benches": [
          {
            "name": "Specific observable construction",
            "value": 0.72182,
            "range": "± 0.387192",
            "unit": "ns",
            "extra": "100 samples\n86534 iterations"
          },
          {
            "name": "Dynamic observable construction",
            "value": 82.0829,
            "range": "± 1.32568",
            "unit": "ns",
            "extra": "100 samples\n700 iterations"
          },
          {
            "name": "Specific observable subscribe",
            "value": 99.301,
            "range": "± 1.08616",
            "unit": "ns",
            "extra": "100 samples\n588 iterations"
          },
          {
            "name": "Dynamic observable subscribe",
            "value": 200.371,
            "range": "± 73.6643",
            "unit": "ns",
            "extra": "100 samples\n318 iterations"
          },
          {
            "name": "OnNext",
            "value": 2.01656,
            "range": "± 0.0414974",
            "unit": "ns",
            "extra": "100 samples\n28803 iterations"
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
          "id": "53f49c43fd812730fdfd6645f518dc66c0cb5b75",
          "message": "Simplify traits a bit",
          "timestamp": "2022-02-27T21:42:14+03:00",
          "tree_id": "e824b0e7ad1e260569d8d60d4cf539aa54e6c1f4",
          "url": "https://github.com/victimsnino/ReactivePlusPlus/commit/53f49c43fd812730fdfd6645f518dc66c0cb5b75"
        },
        "date": 1645987503137,
        "tool": "catch2",
        "benches": [
          {
            "name": "Specific observable construction",
            "value": 0.805556,
            "range": "± 0.0145853",
            "unit": "ns",
            "extra": "100 samples\n38916 iterations"
          },
          {
            "name": "Dynamic observable construction",
            "value": 98.1939,
            "range": "± 2.51123",
            "unit": "ns",
            "extra": "100 samples\n294 iterations"
          },
          {
            "name": "Specific observable subscribe",
            "value": 121.434,
            "range": "± 24.3202",
            "unit": "ns",
            "extra": "100 samples\n256 iterations"
          },
          {
            "name": "Dynamic observable subscribe",
            "value": 218.38,
            "range": "± 6.26229",
            "unit": "ns",
            "extra": "100 samples\n142 iterations"
          },
          {
            "name": "OnNext",
            "value": 2.3485,
            "range": "± 0.85893",
            "unit": "ns",
            "extra": "100 samples\n14993 iterations"
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
          "id": "bee9386d4edc3ee0afad0e1707305d2b9408dd62",
          "message": "Simplify type-traits",
          "timestamp": "2022-02-27T22:03:35+03:00",
          "tree_id": "74f3b796ba5e9e508445ab7ea91e0d2d09c851fb",
          "url": "https://github.com/victimsnino/ReactivePlusPlus/commit/bee9386d4edc3ee0afad0e1707305d2b9408dd62"
        },
        "date": 1645988765437,
        "tool": "catch2",
        "benches": [
          {
            "name": "Specific observable construction",
            "value": 0.881064,
            "range": "± 0.885306",
            "unit": "ns",
            "extra": "100 samples\n37205 iterations"
          },
          {
            "name": "Dynamic observable construction",
            "value": 92.25,
            "range": "± 10.8377",
            "unit": "ns",
            "extra": "100 samples\n304 iterations"
          },
          {
            "name": "Specific observable subscribe",
            "value": 117.893,
            "range": "± 24.5442",
            "unit": "ns",
            "extra": "100 samples\n242 iterations"
          },
          {
            "name": "Dynamic observable subscribe",
            "value": 204.745,
            "range": "± 8.17769",
            "unit": "ns",
            "extra": "100 samples\n137 iterations"
          },
          {
            "name": "OnNext",
            "value": 2.23241,
            "range": "± 0.0343534",
            "unit": "ns",
            "extra": "100 samples\n12297 iterations"
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
          "id": "f06e8f6c4f607a482466adf37ad4c6f25762f486",
          "message": "Small cleanup",
          "timestamp": "2022-02-27T22:16:44+03:00",
          "tree_id": "e801d2cb5f949f26ea647a346a2607b32cd84f25",
          "url": "https://github.com/victimsnino/ReactivePlusPlus/commit/f06e8f6c4f607a482466adf37ad4c6f25762f486"
        },
        "date": 1645989552335,
        "tool": "catch2",
        "benches": [
          {
            "name": "Specific observable construction",
            "value": 0.79952,
            "range": "± 0.38202",
            "unit": "ns",
            "extra": "100 samples\n35804 iterations"
          },
          {
            "name": "Dynamic observable construction",
            "value": 100.675,
            "range": "± 52.1002",
            "unit": "ns",
            "extra": "100 samples\n286 iterations"
          },
          {
            "name": "Specific observable subscribe",
            "value": 126.325,
            "range": "± 57.6997",
            "unit": "ns",
            "extra": "100 samples\n249 iterations"
          },
          {
            "name": "Dynamic observable subscribe",
            "value": 211.118,
            "range": "± 96.6114",
            "unit": "ns",
            "extra": "100 samples\n144 iterations"
          },
          {
            "name": "OnNext",
            "value": 2.38761,
            "range": "± 1.15974",
            "unit": "ns",
            "extra": "100 samples\n12590 iterations"
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
          "id": "1d0d8cf57cf72cc4170d013c4a2732e77ded8064",
          "message": "small fix",
          "timestamp": "2022-02-27T22:35:29+03:00",
          "tree_id": "9af0e3214116d7228360d2993f7ba12d07febaf2",
          "url": "https://github.com/victimsnino/ReactivePlusPlus/commit/1d0d8cf57cf72cc4170d013c4a2732e77ded8064"
        },
        "date": 1645990680789,
        "tool": "catch2",
        "benches": [
          {
            "name": "Specific observable construction",
            "value": 1.00499,
            "range": "± 1.94873",
            "unit": "ns",
            "extra": "100 samples\n38441 iterations"
          },
          {
            "name": "Dynamic observable construction",
            "value": 95.19,
            "range": "± 8.25883",
            "unit": "ns",
            "extra": "100 samples\n300 iterations"
          },
          {
            "name": "Specific observable subscribe",
            "value": 120.953,
            "range": "± 11.1403",
            "unit": "ns",
            "extra": "100 samples\n254 iterations"
          },
          {
            "name": "Dynamic observable subscribe",
            "value": 219.909,
            "range": "± 8.4286",
            "unit": "ns",
            "extra": "100 samples\n143 iterations"
          },
          {
            "name": "OnNext",
            "value": 2.44851,
            "range": "± 3.12995",
            "unit": "ns",
            "extra": "100 samples\n15663 iterations"
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
          "id": "0bd4287bfb7eb80cbcc5c568efc63d1c257259b2",
          "message": "Split subscriber types (#15)",
          "timestamp": "2022-03-02T23:43:31+03:00",
          "tree_id": "b34bce5f5def265d5ea6ad1179dbf62aac5b4e29",
          "url": "https://github.com/victimsnino/ReactivePlusPlus/commit/0bd4287bfb7eb80cbcc5c568efc63d1c257259b2"
        },
        "date": 1646253949918,
        "tool": "catch2",
        "benches": [
          {
            "name": "Specific observable construction",
            "value": 0.592714,
            "range": "± 0.00944178",
            "unit": "ns",
            "extra": "100 samples\n38020 iterations"
          },
          {
            "name": "Dynamic observable construction",
            "value": 72.3955,
            "range": "± 1.49476",
            "unit": "ns",
            "extra": "100 samples\n311 iterations"
          },
          {
            "name": "Specific observable subscribe",
            "value": 102.955,
            "range": "± 2.7911",
            "unit": "ns",
            "extra": "100 samples\n223 iterations"
          },
          {
            "name": "Dynamic observable subscribe",
            "value": 185.718,
            "range": "± 7.24416",
            "unit": "ns",
            "extra": "100 samples\n124 iterations"
          },
          {
            "name": "OnNext",
            "value": 1.47965,
            "range": "± 0.024754",
            "unit": "ns",
            "extra": "100 samples\n15286 iterations"
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
          "id": "6c2ce1eced8dc5fc7514eec78c73007f43425010",
          "message": "Try to use cache (#16)",
          "timestamp": "2022-03-04T00:33:56+03:00",
          "tree_id": "d39db99766b95bdacd1f96d25f147d426d08ab87",
          "url": "https://github.com/victimsnino/ReactivePlusPlus/commit/6c2ce1eced8dc5fc7514eec78c73007f43425010"
        },
        "date": 1646343387012,
        "tool": "catch2",
        "benches": [
          {
            "name": "Specific observable construction",
            "value": 0.671474,
            "range": "± 0.0232753",
            "unit": "ns",
            "extra": "100 samples\n38344 iterations"
          },
          {
            "name": "Dynamic observable construction",
            "value": 81.8806,
            "range": "± 2.23389",
            "unit": "ns",
            "extra": "100 samples\n310 iterations"
          },
          {
            "name": "Specific observable subscribe",
            "value": 115.955,
            "range": "± 3.57046",
            "unit": "ns",
            "extra": "100 samples\n220 iterations"
          },
          {
            "name": "Dynamic observable subscribe",
            "value": 208.862,
            "range": "± 9.39273",
            "unit": "ns",
            "extra": "100 samples\n123 iterations"
          },
          {
            "name": "OnNext",
            "value": 1.76981,
            "range": "± 0.798644",
            "unit": "ns",
            "extra": "100 samples\n15344 iterations"
          }
        ]
      }
    ]
  }
}