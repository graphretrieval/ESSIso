# MQO
## Compile
```
cd test
./build.sh
```
## Usage

```bash
./a.out [OPTION...] -dg DATAGRAPH_FILE -qg QUERYGRAPH_FILE -subIso TURBOISO -queryProTest -maxrc -1 -nresult MAX_NUMBER_OF_RESULTS  -out temp.txt -tout TIMEOUT(s) -batch BATCH_SIZE
```

|Param|Description|
| ------- | ------ |
|-mqo | Multi-query Processing |
|-sqp | Sequential Query Processing |
|-tout TIMEOUT| Specify timeout (seconds)|
|-batch BATCH_SIZE | Specify batch size (MQO) |
|-maxrc NUMBER_OF_CALL | Specify max number of calls, set -1 for infinite |
|-tout TIMEOUT| Specify timeout (seconds)|
|-nresult NUMBEF_OF_RESULTS | Specify max number of results|
|-node | Use node embedding|

### Examples
- Baseline MQO for streaming:`./a.out -dg ../../data/yeast_10_10/d.graph -qg ../../data/yeast_10_10/q0.graph -subIso TURBOISO -queryProTest -mqo -maxrc -1 -nresult 100  -out temp.txt -tout 0.1 -batch 20`
- TurboISO without node embedding:`./a.out -dg ../../data/yeast_10_10/d.graph -qg ../../data/yeast_10_10/q0.graph -subIso TURBOISO -queryProTest -sqp -maxrc 100 -nresult 10  -out temp.txt -tout 60`
- TurboISO with node embedding:`./a.out -dg ../../data/yeast_10_10/d.graph -qg ../../data/yeast_10_10/q0.graph -subIso TURBOISO -queryProTest -sqp -maxrc 100 -nresult 10  -out temp.txt -tout 60 -node`

