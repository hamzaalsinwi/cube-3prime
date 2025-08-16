
# cube_cubeprime_fast

## Overview
This program tests the  **Cube-Three-Prime Conjecture**:that every integer cube `n³` (for `n > 2`)
can be expressed as the sum of **three distinct prime numbers**:

```
n³ = p + q + r with p, q, r prime, p ≠ q ≠ r
```

The code is tuned to finish the range **3 → 20 000** in a few seconds on a modern
quad‑core CPU. It has been successfully tested up to **n ≤ 2 642 245**, the largest value that keeps
`n³` inside the unsigned 64‑bit range.

### Observations

While testing all odd cubes in the range "3 ≤ n ≤ 2,642,245", we found that every cube had at least one valid representation as the sum of three distinct primes that included the number "3".

That is, for each such case, there existed at least one instance where

"n³ = p + q + r" with "{p, q, r} ⊇ {3}"

This consistent pattern suggests that `"3"` plays a foundational role in representing large odd cubes — likely due to being the smallest odd prime, making it a natural anchor in satisfying both the parity and size required for valid prime triplets.

---

## Algorithm

1. **Prime sieve** – A compact byte‑array sieve of Eratosthenes up to **100 million**
is built once at program start.
* All primes are kept in a vector for fast iteration.
* Primality checks for small numbers reuse this table directly.

2. **Deterministic Miller–Rabin** – For numbers above `10 000 000`, a 7‑base
Miller–Rabin test, proven deterministic for the 64‑bit range, is used.

3. **Search strategy**
* **Phase 0** – If `n³` is even, try the pattern `2 + p + q`.
* **Phase 1** – Double loop over distinct small primes`p`and`q`;               compute `r = n³ − p − q` and test `r` with Miller–Rabin.

4. **Parallelism** – A simple work‑stealing counter distributes consecutive `n`
values to a pool of worker threads (`thread::hardware_concurrency()`).

5. **Output** – Each worker pushes its local results into a shared vector,
which is finally sorted and saved to **results.txt**.

---

## Building

```bash
g++ -std=c++20 -O3 -march=native -pthread cube_cubeprime_fast.cpp -o cube_test
```

* `-O3 -march=native` enables full vectorisation on the host CPU.
* `-pthread` links the C++ thread library.

---

## Usage

```
$ ./cube_test
start n (3-2642245): 3
end n (3-2642245): 20000
done: 19998 lines → results.txt
```

The program asks for **inclusive** start / end values and writes one valid representation (or a “NO REPRESENTATION FOUND” line) to results.tx.

---

## Performance notes

| Metric                     | Details                                                   |
|---------------------------|------------------------------------------------------------|
| **Laptop tested on**      | Lenovo IdeaPad Pro 5 16IRH8 (Intel Core i7, 13th Gen)     |
| **CPU speed**             | ~5.3 GHz turbo / 3.5 GHz base                              |
| **Threads used**          | 20 (full capacity, no thermal/power throttling)           |
| **Tested range**          | From `n = 3` to `n = 2,642,245` (≈2.6 million cubes)       |
| **Execution time**        | ≈ 7.3 minutes                                              |
| **RAM usage**             | Starts ~2–3 GB, peaks at 4.5 GB                            |
| **RAM type**              | 16 GB DDR5                                                 |
| **Alternate benchmark**   | Ryzen 5 (3.5 GHz, 4c/8t): `3 → 20 000` in ≈19.2 seconds     |
| **Memory footprint (sieve)** | ≤ 110 MiB (~100M-byte sieve dominates usage)             |


---

## Verifying the absence of counter-examples

To be sure that every cubic number `n³` in the tested range was successfully expressed as the sum of three distinct primes (or `2 + p + q` for even cubes), check that **no** line in `results.txt` contains the phrase `NO REPRESENTATION FOUND`.

1. Open a terminal in the folder that contains `results.txt`.
2. Run **one** of the following commands, depending on your system:

   * **Linux / macOS**  
```bash
grep -c "NO REPRESENTATION FOUND" results.txt 
```

   * **Windows (PowerShell ≥ 5)**  
```bash 
Select-String -Path results.txt -Pattern "NO REPRESENTATION FOUND" | Measure-Object
```

   * **Windows (cmd.exe legacy)**  
```bash
findstr /C:"NO REPRESENTATION FOUND" results.txt | find /C /V ""
```

3. If the command prints `0`, there are no counter-examples in the file; otherwise the printed number equals the count of failures.  
   To inspect them in detail, save the offending lines with line numbers:
   * **Linux / macOS**
```bash
grep -n "NO REPRESENTATION FOUND" results.txt > counterexamples.txt
```
   * **PowerShell**
```bash
Select-String -Path results.txt -Pattern "NO REPRESENTATION FOUND" | ForEach-Object {
    "$($_.LineNumber),$($_.Line)"
} | Set-Content counterexamples.csv
```

> **Status note:** We have already scanned the full allowable range ( 3 ≤ n ≤ 2 642 245 ) with the current settings (distinct primes, `p, q ≤ 10⁸`) and obtained zero counter-examples. The steps above are provided for clarity and for anyone who reruns the experiment under different parameters.

---

## Limitations & Future work


* The sieve limit is fixed at *100 million*. Extending above that will
increase memory linearly. A segmented sieve or wheel optimization would
scale better.
* **Phase 1** scans `O(π(N)²)` prime pairs; heuristics or meet‑in‑the‑middle
techniques could prune the search further.
* The Miller–Rabin tester could switch to a **strong Lucus test** for slightly
faster large‑prime rejection.
* Extending the search to the full 128-bit domain (i.e., `n³ < 2¹²⁸`, so `n ≤ ≈ 7 × 10¹²`) is arithmetically straightforward with 128-bit limbs, and ranges of a few million `n` remain tractable on contemporary CPUs; however, covering the entire space would need massive parallelism—porting the tight inner loops to CUDA/OpenCL and running on multi-GPU hardware (or GPU clusters) is the most practical way to achieve the required throughput.

---
### For the human and moral dedication behind this project, see: [README_gaza.md](./README_gaza.md)

---

### Conjecture: Every integer cube n³ (n > 2) = p + q + r with distinct primes

Author: Generated by Mr. Hamza Alsenwi & ChatGPT (OpenAI o3 & 4o), (My assistant and companion, even if just an LLM)

– 29 June 2025

**License:** MIT – feel free to use and modify.
