# GraphSearch
## Compile
```
./build.sh
```
## Usage

```bash
./out [OPTION...] DATAGRAPH_FILE QUERYGRAPH_DIR [COLDSTART_QUERYGRAPH_DIR]
```
|Param|Description|
| ------- | ------ |
|-a NUMBER_OF_CALL | Specify max number of calls|
|-t TIMEOUT| Specify timeout (seconds)|
|-e NUMBEF_OF_RESULTS | Specify max number of results|
|-c | Use cache |
|-s CACHE_SIZE | Cache size|
|-f | Use LFU|
|-r | Use LRU|
|-d | Use distance utility|
|-h | Consider case 2 |
|-v | Use VF2 algorithm or TurboIs (default) |

### Examples
- Baseline VF2: `./out -t 0.1 -e 100 -v ../data/yeast_10_10/d.dimas ../data/yeast_10_10/query0/`
- Baseline TurboIso: `./out -t 0.1 -e 100 ../data/yeast_10_10/d.dimas ../data/yeast_10_10/query0/`
- LRU TurboIso: `./out -t 0.1 -e 100 -c -s 20 -r ../data/yeast_10_10/d.dimas ../data/yeast_10_10/query0/`
- LFU TurboIso: `./out -t 0.1 -e 100 -c -s 20 -f ../data/yeast_10_10/d.dimas ../data/yeast_10_10/query0/`
- Recache TurboIso: `./out -t 0.1 -e 100 -c -s 20  ../data/yeast_10_10/d.dimas ../data/yeast_10_10/query0/`
- FaSS TurboIso: `./out -t 0.1 -e 100 -c -s 20 -d ../data/yeast_10_10/d.dimas ../data/yeast_10_10/query0/`
- FaSS TurboIso Coldstart: `./out -t 0.1 -e 100 -c -s 100 -d ../data/yeast_10_10/d.dimas ../data/yeast_10_10/query0/ ../data/yeast_15_coldstart/query/`
