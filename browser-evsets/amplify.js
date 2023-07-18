const AMP_ITERATIONS = 17;
const AMP_TRAVERSE = 7100;

const EV_L1 = 8;
const NUMBER_CONGRUENT_L1 = 16;

const EVICT_LLC_SIZE = 128 * 1024 * 1024;

const s_evsetL1_buf = new ArrayBuffer(EVICT_LLC_SIZE);
const s_evsetL1 = new Uint32Array(s_evsetL1_buf);

const s_evsetL1_c_buf = new ArrayBuffer(EVICT_LLC_SIZE);
const s_evsetL1_c = new Uint32Array(s_evsetL1_c_buf);

const evict_mem_L1_buf = new ArrayBuffer(EVICT_LLC_SIZE);
const evict_mem_L1 = new Uint32Array(evict_mem_L1_buf);

const evict_mem_L1_c_buf = new ArrayBuffer(EVICT_LLC_SIZE);
const evict_mem_L1_c = new Uint32Array(evict_mem_L1_c_buf);

ps_evset_premap(evict_mem_L1_buf);
ps_evset_premap(evict_mem_L1_c_buf);

function ps_evset_premap(page) {
    let i;
    for (i = 0; i < EVICT_LLC_SIZE / 8; i += 128)
        page[i] = 1;

    for (i = 0; i < EVICT_LLC_SIZE / 8; i += 128)
        page[i] = 0;
}

function construct_L1_eviction_set(evset, page, target, SIZE, STRIDE) {
    for (let i = 0; i < SIZE; i++) {
        evset[i] = target + i * STRIDE / 4;
    }
}

export function amplify(target_index, ptr, view, action, div) {
    const LAD_RESULTS = new Array(AMP_ITERATIONS);
    const PERF_RESULTS = new Array(AMP_ITERATIONS);
    let j0 = 0, j3 = 0, j4 = 0;

    // For the measurement, use an L1 set to which the target does not map

    construct_L1_eviction_set(s_evsetL1, evict_mem_L1, target_index + 1187, NUMBER_CONGRUENT_L1, 4096);
    construct_L1_eviction_set(s_evsetL1_c, evict_mem_L1_c, target_index + 1187, NUMBER_CONGRUENT_L1, 4096);

    // Set everything to zero (for the data dependency trix)
    for (let i = 0; i < NUMBER_CONGRUENT_L1; i++) { evict_mem_L1[s_evsetL1[i]] = 0; }
    for (let i = 0; i < NUMBER_CONGRUENT_L1; i++) { evict_mem_L1_c[s_evsetL1_c[i]] = 0; }
    evict_mem_L1_c[target_index + 387] = 0;

    //////////////////////////////////////////////
    // Convert LLC state difference to L1 state difference, for use with the L1 amplifier
    if (action === 2) {
        let hop_nb = 0;
        let start = performance.now(j0); j0 &= start;
        for (let ITERATOR = j0; ITERATOR < AMP_ITERATIONS; ITERATOR++) {

            j0 = view.getUint32(target_index ^ (j0 & ITERATOR), true);

            // sprinkle data dependencies everywhere to ensure correct ordering of things
            for (let nb_traversals = 0; nb_traversals < 3; nb_traversals++) {
                for (let index = ptr + j0; index != 0; index = view.getUint32(index, true)) { j0 &= index; }
            }
            j0 = evict_mem_L1[(target_index + 387) ^ j0];             // Access that is slow for frontend to catch up
            j0 = view.getUint32((target_index + 117) ^ j0, true);   // Ensure TLB hit for the target

            const start = performance.now(j0); j0 &= start;

            for (let i = j0; i < EV_L1; i++) { j0 = evict_mem_L1_c[s_evsetL1_c[i] + j0]; } // evict L1 first

            // Access pattern to produce (AB-CD)-(DE-FG) PLRU tree
            j0 = evict_mem_L1[s_evsetL1[0] + j0]; j0 = evict_mem_L1[s_evsetL1[1] + j0]; j0 = evict_mem_L1[s_evsetL1[2] + j0]; j0 = evict_mem_L1[s_evsetL1[7] + j0]; j0 = evict_mem_L1[s_evsetL1[5] + j0]; j0 = evict_mem_L1[s_evsetL1[6] + j0]; j0 = evict_mem_L1[s_evsetL1[4 + j0]]; j0 = evict_mem_L1[s_evsetL1[3]] + j0;

            //////////////// LLC status into L1////////////////////////////////////////////////////////////////////
            // Convert: Turn replacement status through reordering
            j3 = view.getUint32(target_index ^ j0, true);         // The target line access we want to measure
            j3 = evict_mem_L1[s_evsetL1[4] + j3];                 // Access to D

            // 40x multiplication before H is accessed
            j0 = j0 * 1700; j0 = j0 * 1700; j0 = j0 * 1700; j0 = j0 * 1700; j0 = j0 * 1700;
            j0 = j0 * 1700; j0 = j0 * 1700; j0 = j0 * 1700; j0 = j0 * 1700; j0 = j0 * 1700;
            j0 = j0 * 1700; j0 = j0 * 1700; j0 = j0 * 1700; j0 = j0 * 1700; j0 = j0 * 1700;
            j0 = j0 * 1700; j0 = j0 * 1700; j0 = j0 * 1700; j0 = j0 * 1700; j0 = j0 * 1700;
            j0 = j0 * 1700; j0 = j0 * 1700; j0 = j0 * 1700; j0 = j0 * 1700; j0 = j0 * 1700;
            j0 = j0 * 1700; j0 = j0 * 1700; j0 = j0 * 1700; j0 = j0 * 1700; j0 = j0 * 1700;

            j4 = evict_mem_L1[s_evsetL1[3] + j0];                // Access to H

            // If the leg containing the access to the target wins: D then H. This means
            j0 = evict_mem_L1[s_evsetL1[8] ^ j4 ^ j3];
            ////////////////////////////////////////////////////////////////////////////////////

            // Interlude
            j0 = evict_mem_L1[s_evsetL1[6] + j0]; j0 = evict_mem_L1[s_evsetL1[3] + j0]; j0 = evict_mem_L1[s_evsetL1[5] + j0];

            for (let yit = 0; yit < AMP_TRAVERSE; yit++) {
                /*
                    Traversal: use a distance-2 pattern for robustness
                */
                j0 = evict_mem_L1[s_evsetL1[0] + j0]; // A
                j0 = evict_mem_L1[s_evsetL1[1] + j0]; // E
                j0 = evict_mem_L1[s_evsetL1[5] + j0]; // B
                j0 = evict_mem_L1[s_evsetL1[2] + j0]; // C
                j0 = evict_mem_L1[s_evsetL1[7] + j0]; // G
                j0 = evict_mem_L1[s_evsetL1[5] + j0]; // B
                j0 = evict_mem_L1[s_evsetL1[4] + j0]; // D
                j0 = evict_mem_L1[s_evsetL1[6] + j0]; // F
                j0 = evict_mem_L1[s_evsetL1[5] + j0]; // B
                j0 = evict_mem_L1[s_evsetL1[1] + j0]; // E
                j0 = evict_mem_L1[s_evsetL1[3] + j0]; // H
                j0 = evict_mem_L1[s_evsetL1[5] + j0]; // B
                j0 = evict_mem_L1[s_evsetL1[7] + j0]; // G
                j0 = evict_mem_L1[s_evsetL1[0] + j0]; // A
                j0 = evict_mem_L1[s_evsetL1[5] + j0]; // B
                j0 = evict_mem_L1[s_evsetL1[6] + j0]; // F
                j0 = evict_mem_L1[s_evsetL1[2] + j0]; // C
                j0 = evict_mem_L1[s_evsetL1[5] + j0]; // B
                j0 = evict_mem_L1[s_evsetL1[3] + j0]; // H
                j0 = evict_mem_L1[s_evsetL1[4] + j0]; // D
                j0 = evict_mem_L1[s_evsetL1[5] + j0]; // B

                // Refresh pattern
                if (yit & 0xF == 0) {
                    j0 = evict_mem_L1[s_evsetL1[5] + j0]; // B
                    j0 = evict_mem_L1[s_evsetL1[5] + j0]; // B
                    j0 = evict_mem_L1[s_evsetL1[5] + j0]; // B
                    j0 = evict_mem_L1[s_evsetL1[5] + j0]; // B
                    j0 = evict_mem_L1[s_evsetL1_c[0] + j0]; //
                    j0 = evict_mem_L1[s_evsetL1_c[1] + j0]; //
                    j0 = evict_mem_L1[s_evsetL1[5] + j0]; // B
                    j0 = evict_mem_L1[s_evsetL1_c[2] + j0]; //
                    j0 = evict_mem_L1[s_evsetL1_c[3] + j0]; //
                    j0 = evict_mem_L1[s_evsetL1[5] + j0]; // B
                    j0 = evict_mem_L1[s_evsetL1_c[4] + j0]; //
                    j0 = evict_mem_L1[s_evsetL1[5] + j0]; // B
                    j0 = evict_mem_L1[s_evsetL1_c[5] + j0]; //

                    j0 = evict_mem_L1[s_evsetL1[1] + j0]; // E
                    j0 = evict_mem_L1[s_evsetL1[2] + j0]; // C
                    j0 = evict_mem_L1[s_evsetL1[5] + j0]; // B
                    j0 = evict_mem_L1[s_evsetL1[7] + j0]; // G
                    j0 = evict_mem_L1[s_evsetL1[4] + j0]; // D
                    j0 = evict_mem_L1[s_evsetL1[5] + j0]; // B
                    j0 = evict_mem_L1[s_evsetL1[6] + j0]; // F
                    j0 = evict_mem_L1[s_evsetL1[5] + j0]; // B
                    j0 = evict_mem_L1[s_evsetL1[3] + j0]; // H
                    j0 = evict_mem_L1[s_evsetL1[5] + j0]; // B

                }

            }

            const end = performance.now(j0) - start;
            LAD_RESULTS[ITERATOR ^ j0] = end;
            PERF_RESULTS[ITERATOR ^ j0] = end;
        }
    }

    return { perf: PERF_RESULTS, counter: LAD_RESULTS };
}
