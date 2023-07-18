import { amplify } from './amplify.js';

// Statistics
export function stats(data) {
    return {
        'min': Math.min.apply(0, data),
        'max': Math.max.apply(0, data),
        'mean': mean(data),
        'median': median(data),
        'std': std(data),
        'mode': mode(data),
        'toString': function () {
            return `{min: ${this.min.toFixed(2)},\tmax: ${this.max.toFixed(2)},\tmean: ${this.mean.toFixed(2)},\tmedian: ${this.median.toFixed(2)},\tstd: ${this.std.toFixed(2)}}`;
        }
    };
}

export function min(arr) {
    return Math.min.apply(0, arr);
}

export function mean(arr) {
    return arr.reduce((a, b) => a + b) / arr.length;
}

export function median(arr) {
    arr.sort((a, b) => a - b);
    return (arr.length % 2) ? arr[(arr.length / 2) | 0] : mean([arr[arr.length / 2 - 1], arr[arr.length / 2]]);
}

export function mode(arr) {
    var counter = {};
    var mode = [];
    var max = 0;
    for (var i in arr) {
        if (!(arr[i] in counter)) {
            counter[arr[i]] = 0;
        }
        counter[arr[i]]++;
        if (counter[arr[i]] == max) {
            mode.push(arr[i]);
        } else if (counter[arr[i]] > max) {
            max = counter[arr[i]];
            mode = [arr[i]];
        }
    }
    return mode;
}

function variance(arr) {
    var x = mean(arr);
    return arr.reduce((pre, cur) => pre + ((cur - x) ** 2)) / (arr.length - 1);
}

function std(arr) {
    return Math.sqrt(variance(arr));
}

// Overload
Function.prototype.toSource = function () {
    return this.toString().slice(this.toString().indexOf('{') + 1, -1);
}

Object.defineProperty(Array.prototype, 'chunk', {
    value: function (i, n) {
        let results = [];
        let ceiled = this.length % n;
        let k = Math.ceil(this.length / n);
        let q = Math.floor(this.length / n);
        let c = 0;
        for (i = 0; i < ceiled; i++) {
            results[i] = this.slice(c, c + k);
            c += k;
        }
        for (i; i < n; i++) {
            results[i] = this.slice(c, c + q);
            c += q;
        }
        return results;
    }
});

const BM = 128 * 1024 * 1024; // Eviction buffer
const WP = 64 * 1024; // A WebAssembly page has a constant size of 64KB
const SZ = BM / WP; // 128 hardcoded value in wasm

// Shared memory
export const memory = new WebAssembly.Memory({ initial: SZ, maximum: SZ, shared: true });

let hit = 0;
let miss = 0;
let threshold = 0;

let timerConfig = {
    measureHit: (victim, view) => amplify(victim, 0, view, 2, 4).counter,
    measureMiss: (victim, pointer, view) => amplify(victim, pointer, view, 2, 4).counter,
    calculateThreshold: (hitMedian, missMedian) => {
        hit = hitMedian;
        miss = missMedian;
        if (missMedian + 0.03 > hitMedian) {
            threshold = 0;
        } else {
            threshold = (Number(hitMedian) + Number(missMedian) * 2) / 3;
        }
        return threshold;
    },
    getMiss: (yes) => {
        return miss;
    },
    determineHit: (measurement, nb, lowest) => {
        return ((measurement > (lowest * 1.20)) ? "hit" : "miss");
    },
}

function EvSet(view, nblocks, start = 8192, victim = 4096, assoc = 16, stride = 4096, offset = 0) {
    const RAND = true;

    /* private methods */
    this.genIndices = function (view, stride) {
        let arr = [], j = 0;
        for (let i = (stride) / 4; i < (view.byteLength - this.victim) / 4; i += stride / 4) {
            arr[j++] = this.victim + i * 4;
            view.setUint32(arr[j - 1], 0, true);
        }
        return arr;
    }

    this.randomize = function (arr) {
        console.log("randomizing indices")
        for (let i = arr.length; i; i--) {
            var j = Math.floor(Math.random() * i | 0) | 0;
            [arr[i - 1], arr[j]] = [arr[j], arr[i - 1]];
        }
        return arr;
    }

    this.indicesToLinkedList = function (buf, indices) {
        if (indices.length == 0) {
            this.ptr = 0;
            return;
        }
        let pre = this.ptr = indices[0];
        for (let i = 1; i < indices.length; i++) {
            view.setUint32(pre, indices[i], true);
            pre = indices[i];
        }
        view.setUint32(pre, 0, true);
    }

    this.init = function () {
        let indx = this.genIndices(view, stride);
        if (RAND) indx = this.randomize(indx);
        indx.splice(nblocks, indx.length); // select nblocks elements
        this.indicesToLinkedList(view, indx);
        return indx;
    }
    /* end-of-private */

    /* properties */
    this.start = start;
    this.offset = (offset & 0x3f) << 6;
    //this.victim = victim + this.offset;
    this.victim = victim + 128 * stride;
    this.initialVictim = this.victim;
    view.setUint32(this.victim, 0, true); // lazy alloc
    this.assoc = assoc;
    this.ptr = 0;
    this.refs = this.init();
    this.del = [];
    this.vics = [];
    /* end-of-properties */

    /* public methods */
    this.unlinkChunk = function unlinkChunk(chunk) {
        let s = this.refs.indexOf(chunk[0]), f = this.refs.indexOf(chunk[chunk.length - 1]);
        view.setUint32(this.refs[f], 0, true);
        this.refs.splice(s, chunk.length); // splice chunk indexes
        if (this.refs.length === 0) { // empty list
            this.ptr = 0;
        } else if (s === 0) { // removing first chunk
            this.ptr = this.refs[0];
        } else if (s > this.refs.length - 1) { // removing last chunk
            view.setUint32(this.refs[this.refs.length - 1], 0, true);
        } else { // removing middle chunk
            view.setUint32(this.refs[s - 1], this.refs[s], true);
        }
        this.del.push(chunk); // right
    }

    this.relinkChunk = function relinkChunk() {
        let chunk = this.del.pop(); // right
        if (chunk === undefined) {
            return;
        }
        this.ptr = chunk[0];
        if (this.refs.length > 0) {
            view.setUint32(chunk[chunk.length - 1], this.refs[0], true);
        }
        if (typeof (chunk) === 'number') {
            this.refs.unshift(chunk); // left
        } else {
            this.refs.unshift(...chunk); // left
        }
    }

    this.groupReduction = function groupReduction(miss, determineHit, getMiss) {
        let agree = 0, misshit = 0, hitmiss = 0, fail = 0, hitc = 0, missc = 0;
        const MAX = 100;
        let i = 0, r = 0;
        let notFoundCounter = 0;
        let lowest = getMiss() * 0.95;
        let atl = lowest;
        while (this.refs.length > this.assoc) {
            let m = this.refs.chunk(i, this.assoc + 1);
            let found = false;
            let cnt = 0;
            for (let c in m) {

                // Remove the chunk from the list
                this.unlinkChunk(m[c]);

                // Results with crappy timer
                let measurements = miss(this.victim, this.ptr)
                let t = mean(measurements);

                let res = determineHit(t, this.refs.length, lowest);
                let tmax = stats(measurements).max;
                let resmax = determineHit(tmax);

                // Only categorize as 'miss' if sufficient evidence
                let votes = 1;
                let votesMiss = 1;
                let missThreshold = (this.refs.length > 50) ? ((this.refs.length > 1000) ? 20 : 20) : 25;
                while (res == 'miss' && votes < missThreshold) {
                    measurements = miss(this.victim, this.ptr)
                    t = mean(measurements);
                    res = determineHit(t, this.refs.length, lowest);
                    if (res == "miss") {
                        votesMiss++;
                    }
                    votes++;
                    if (votes - votesMiss > 1) {
                        res = "fail;";
                    }
                }
                if (res == "miss") {
                    votesMiss++;
                    if (t < lowest) {
                        lowest = lowest - 0.2 * (lowest - t); atl = (lowest < atl) ? lowest : atl;
                    } else {
                        lowest = (lowest > (atl + 0.15)) ? lowest : lowest + 0.02 * (t - lowest);
                    }
                }

                // Bookkeeping
                if (res == 'hit') hitc++;
                if (res == 'miss') missc++;
                if (res == 'hit' || res == "fail") {
                    this.relinkChunk();
                } else {
                    found = true;
                    notFoundCounter = 0;
                    console.log(this.refs.length, t, res, lowest);
                    break;
                }
            }
            if (!found) {
                if (notFoundCounter > 0) {
                    notFoundCounter = 0;
                    r += 1;
                    if (r < MAX) {
                        this.relinkChunk();
                        if (this.del.length === 0) break;
                    } else {
                        while (this.del.length > 0) {
                            this.relinkChunk();
                        }
                        break;
                    }
                } else {
                    notFoundCounter++;
                }
            }
        }
        console.log(`total hits: ${hitc}, total misses: ${missc}`)
    }

    this.relink = function () {
        this.indicesToLinkedList(this.buffer, this.refs);
    }
    /* end-of-public */
}

function finderMsg(msg) {
    switch (msg.type) {
        case 'log':
            log(...msg.str);
            console.log(...msg.str);
            msg.str.map(e => % DebugPrint(e)); // used for verification
            break;
        case 'eof':
            console.log("end:", performance.now());
        default:
    }
};

function finderLog(...args) {
    finderMsg({ type: 'log', str: args });
}

// Constants
export const VERBOSE = true;
const NOLOG = false;

let THRESHOLD = 0;
const RESULTS = [];

let startTime = 0;
let endTime = 0;

const CALI_LIMIT = 5;

export async function finderStart(evt) {
    // Parse settings
    let { B, VICTIM, OFFSET, ASSOC, STRIDE, LIMIT } = evt.conf;

    let timerConfig = evt.timerConfig;

    // Prepare wasm instance
    const { memory } = evt;
    // Memory view
    const view = new DataView(memory.buffer);

    if (!NOLOG) finderLog('Prepare new evset');
    const evset = new EvSet(view, B, VICTIM * 2, VICTIM, ASSOC, STRIDE, OFFSET);

    const RETRY = 10;
    await new Promise(r => setTimeout(r, 10)); // timeout to allow counter


    const wasmMeasureOpt = {
        hit: function hit(vic) {
            return timerConfig.measureHit(vic, view);
        },
        miss: function miss(vic, ptr) {
            return timerConfig.measureMiss(vic, ptr, view);
        }
    }

    let i = 0;
    while (i++ < CALI_LIMIT) {
        THRESHOLD = runCalibration('Wasm measure opt', evset, wasmMeasureOpt.hit, wasmMeasureOpt.miss, timerConfig.calculateThreshold);
        console.log("real threshold:", THRESHOLD);
        if (THRESHOLD) {
            break;
        } else {
            evset.refs = evset.init();
        }
    }

    if (THRESHOLD == 0) {
        finderLog('Calibration error');
        finderMsg({ type: 'eof', results: RESULTS, victim: evset.victim });
        evt.callback(RESULTS, evset.victim);
        return RESULTS;
    }

    finderLog('Calibrated threshold: ' + THRESHOLD);
    startTime = parseInt(new Date().getTime() / 1000);

    do {
        let r = 0;
        while (!cb(evset, wasmMeasureOpt.miss, timerConfig) && ++r < RETRY && evset.victim) {
            if (VERBOSE) finderLog('retry');
        }

        if (r < RETRY) {
            RESULTS.push(evset.refs); // save eviction set
            evset.refs = evset.del.slice();
            evset.del = [];
            evset.relink(); // from new refs
            if (VERBOSE) finderLog('Find next (', evset.refs.length, ')');
        }
    } while (evset.refs.length > ASSOC && RESULTS.length < LIMIT);

    finderLog('Found ' + RESULTS.length + ' different eviction sets');
    finderLog('EOF');
    finderMsg({ type: 'eof', results: RESULTS, victim: evset.victim });
    evt.callback(RESULTS, evset.victim);
    return RESULTS;
}

function runCalibration(title, evset, hit, miss, calculateThreshold) {
    const T = 20;
    const CALI_MULTI = 100;

    const VICTIM = evset.victim | 0;
    const PTR = evset.ptr | 0;

    console.log('calibrate')
    for (let i = 0; i < T; i++) {
        hit(VICTIM);
        miss(VICTIM, 0);
    }
    // real run
    let t_hit = [];
    let t_miss = [];
    for (let i = 0; i < CALI_MULTI; i++) {
        t_hit = t_hit.concat(hit(VICTIM));
        t_miss = t_miss.concat(miss(VICTIM, PTR));
    }
    // output
    if (VERBOSE) finderLog('--- ' + title + ' ---');
    if (VERBOSE) finderLog('Hit:\t' + (Array.isArray(t_hit) ? stats(t_hit) : t_hit));
    if (VERBOSE) finderLog('Miss:\t' + (Array.isArray(t_miss) ? stats(t_miss) : t_miss));
    if (VERBOSE) finderLog('-----------');

    // calc threshold
    if (Array.isArray(t_hit)) {
        t_hit = stats(t_hit).mean;
    }
    if (Array.isArray(t_miss)) {
        t_miss = stats(t_miss).mean;
    }
    return calculateThreshold(t_hit, t_miss);
}

function cb(evset, miss, timerConfig) {
    if (VERBOSE) finderLog('Starting reduction...');
    evset.groupReduction(miss, timerConfig.determineHit, timerConfig.getMiss);

    if (evset.refs.length === evset.assoc) {
        endTime = parseInt(new Date().getTime() / 1000);
        if (!NOLOG) finderLog('Total time: ', endTime - startTime);
        if (!NOLOG) finderLog('Yo fam, found me a set');
        if (!NOLOG) finderLog('Victim addr: ' + evset.victim);
        if (!NOLOG) finderLog('Eviction set: ' + evset.initialVictim + "," + evset.refs);
        evset.del = evset.del.flat();
        return true;
    } else {
        while (evset.del.length > 0) {
            evset.relinkChunk();
        }
        if (VERBOSE) finderLog('Failed: ' + evset.refs.length);
        return false;
    }
}

export async function start(config, callback) {
    console.log("start:", performance.now());
    return finderStart({ "memory": memory, "conf": config, timerConfig: timerConfig, callback: callback });
}

let ws = document.getElementById("perfstart");
if (ws) {
    ws.onclick = async () => {
        let mode = document.querySelector('input[name="run"]:checked').value;
        let rep = 0;
        let base = async () => start(getConf(), () => { });
        if (mode == 'single') {
            rep = 1;
        } else {
            rep = parseInt(document.getElementById("x").value);
        }
        let functions = new Array(rep);
        functions[0] = base;
        for (let i = 0; i < rep; i++) {
            if (mode == 'rando') { randomizeVictim(); }
            let s = await start(getConf(), () => { });
        }

        return false;
    };
}
