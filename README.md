
# cube_cubeprime_fast

## Overview
This program tests the **cubic cube three‑prime conjecture** that every integer cube `n³` (for `n > 2`)
can be expressed as the sum of **three distinct prime numbers**:

```
n³ = p + q + r with p, q, r prime, p ≠ q ≠ r
```

The code is tuned to finish the range **3 → 20 000** in a few seconds on a modern
quad‑core CPU. It has been successfully tested up to **n ≤ 2 642 245**, the largest value that keeps
`n³` inside the unsigned 64‑bit range.

---

## Algorithm

1. **Prime sieve** – A compact bit‑array sieve of Eratosthenes up to **100 million**
is built once at program start.
* All primes are kept in a vector for fast iteration.
* Primality checks for small numbers reuse this table directly.

2. **Deterministic Miller–Rabin** – For numbers above `10 000 000`, a 7‑base
Miller–Rabin test, proven deterministic for the 64‑bit range, is used.

3. **Search strategy**
* **Phase 0** – If `n³` is even, try the pattern `2 + p + q`.
* **Phase 1** – Double loop over distinct small primes `p` and `q`;
compute `r = n³ − p − q` and test `r` with Miller–Rabin.

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

The program asks for **inclusive** start / end values and writes every
representation (or a “NO REPRESENTATION FOUND” line) to `results.txt`.

---

## Performance notes

* The program was tested on a Lenovo IdeaPad Pro 5 16IRH8 laptop with an ~5.3_3.5 GHz Intel Core i7 (13th Gen) processor, running in High Performance Mode.
* It utilized **20 threads** at full capacity with no thermal, power, or memory throttling.

* Tested range: from 3 to 2,642,245 (≈2.6 million cube values)
* Execution time: **≈ 7.3 minutes**
* Memory usage: started at **~2–3 GB** and peaked at **4.5 GB**
* RAM: 16 GB DDR5
* On a 3.5 GHz Ryzen 5 (4 cores / 8 threads) the default range `3→20 000`
completes in **≈ 19.2s**.
* Memory footprint is **≤ 110 MiB** (dominated by the 100 Mbit sieve).

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
## 🇵🇸 About Gaza

**This project is dedicated to the resilient people of Gaza.**

At the time of writing, they are enduring a **brutal genocide** and **forced starvation** under a relentless siege.

- **Food**, **water**, **medicine**, and **electricity** are deliberately cut off.  
- **Civilians are being targeted**, and even **hospitals and shelters** are not spared.  
- There is no safety — only suffering.

We write this with **heavy hearts**, praying for **justice**, **mercy**, and **freedom** for Gaza.

> _“And your Lord is never forgetful.”_ — **Surah Maryam, 64**

**O Allah, be with the people of Gaza.**  
**O Allah, destroy the oppressors and those who support them.**


---

## Conjecture: Every integer cube n³ (n > 2) = p + q + r with distinct primes

Author: Generated by Mr. Hamza Alsenwi & ChatGPT (OpenAI o3 & 4o), (My assistant and companion, even if just an LLM)

– 29 June 2025

**License:** MIT – feel free to use and modify.