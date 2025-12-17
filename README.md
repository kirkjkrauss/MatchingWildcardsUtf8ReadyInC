# MatchingWildcardsUtf8ReadyInC
UTF-8-ready routines for matching wildcards

Matching Wildcards &ndash; UTF-8-ready &ndash; in C/C++

This file set includes UTF-8-ready routines for matching wildcards in C/C++ (fastwildcompare.cpp), based on the ASCII-only C/C++ implementation here: https://developforperformance.com/MatchingWildcards_AnImprovedAlgorithmForBigData.html

The routines include a length-checking version and a version that checks only for null terminators for the inbound strings.  Also included: ASCII testcases for correctness and performance, plus a new set of UTF-8 testcases originally implemented in Rust.

A description of the algorithm's implementation and testing strategies, along with performance and runtime analysis findings, appear here: https://developforperformance.com/MatchingWildcardsUTF8ReadyInGoSwiftAndCpp.html#MatchingWildcardsInCppNewUTF8readyRoutines
