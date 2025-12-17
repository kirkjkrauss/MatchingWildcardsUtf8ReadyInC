// C/C++ testcases for matching wildcards, including UTF-8 strings.
//
// Copyright 2025 Kirk J Krauss.  This is a Derivative Work based on 
// material that is copyright 2025 Kirk J Krauss and available 
// at
//
//     https://developforperformance.com/MatchingWildcardsInRust.html
// 
// Licensed under the Apache License, Version 2.0 (the "License")
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// 
//     https://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// This file provides sets of correctness and performance tests, for 
// matching wildcards in C/C++, along with a main() routine that invokes 
// the testcases and outputs the results.
//
// File-scope testcase selection flags.
//
// For a fair comparison involving implementations that aren't UTF-8-ready, 
// leave COMPARE_PERFORMANCE undefined.
//
//#define COMPARE_PERFORMANCE       1
//#define COMPARE_CASE_INSENSITIVE  1
#define COMPARE_WILD                1
#define COMPARE_TAME                1
#define COMPARE_EMPTY               1
#define COMPARE_UTF8                1

#include <stdio.h>
#include <string.h>
#include "fastwildcompare.h"

#if defined(COMPARE_PERFORMANCE)
#include <stdint.h>
#include <chrono>
#include <cmath>

// File-scope variables for low-latency accumulation of performance data.
//
uint64_t  iAccumulatedTimeAscii;
uint64_t  iAccumulatedTimeUtf8;
uint64_t  iAccumulatedTimeLenUtf8;
#endif  // COMPARE_PERFORMANCE

// This function compares a tame/wild string pair via each included routine.
//
bool test(char *pTame, char *pWild, bool bExpectedResult)
{
	bool bPassed = true;
    size_t lenWild = CodePointCount(pWild);
    size_t lenTame = CodePointCount(pTame);

#if defined(COMPARE_PERFORMANCE)
    std::chrono::time_point<std::chrono::high_resolution_clock> timeStart;
    std::chrono::time_point<std::chrono::high_resolution_clock> timeFinish;
#endif  // COMPARE_PERFORMANCE

#if !defined(COMPARE_UTF8)
    // Sanity check: An ASCII string's length in bytes should equal the 
    // number of code points in it.
    if (lenWild != strlen(pWild))
    {
        printf("Code point count does not equal string length for:\n%s\n", 
               pWild);
    }

    if (lenTame != strlen(pTame))
    {
        printf("Code point count does not equal string length for:\n%s\n",
            pTame);
    }

#if defined(COMPARE_PERFORMANCE)
    timeStart = std::chrono::high_resolution_clock::now();
#endif  // COMPARE_PERFORMANCE

    if (bExpectedResult != FastWildCompare(pWild, pTame))
    {
        bPassed = false;
    }

#if defined(COMPARE_PERFORMANCE)
    timeFinish = std::chrono::high_resolution_clock::now();
    iAccumulatedTimeAscii += 
        (std::chrono::duration_cast<std::chrono::nanoseconds>(
                                        timeFinish - timeStart)).count();
    timeStart = std::chrono::high_resolution_clock::now();
#endif  // COMPARE_PERFORMANCE
#endif  // !COMPARE_UTF8

	if (bExpectedResult != FastWildCompareUtf8(pWild, pTame))
	{
		bPassed = false;
	}

#if defined(COMPARE_PERFORMANCE)
    timeFinish = std::chrono::high_resolution_clock::now();
    iAccumulatedTimeUtf8 +=
        (std::chrono::duration_cast<std::chrono::nanoseconds>(
            timeFinish - timeStart)).count();
    timeStart = std::chrono::high_resolution_clock::now();
#endif  // COMPARE_PERFORMANCE

	if (bExpectedResult != FastWildLenCompareUtf8(pWild, pTame, 
		                                          lenWild, lenTame))
	{
		bPassed = false;
	}

#if defined(COMPARE_PERFORMANCE)
    timeFinish = std::chrono::high_resolution_clock::now();
    iAccumulatedTimeLenUtf8 +=
        (std::chrono::duration_cast<std::chrono::nanoseconds>(
            timeFinish - timeStart)).count();
#endif  // COMPARE_PERFORMANCE

	return bPassed;
}


// A set of wildcard comparison tests.
//
void testwild(void)
{
    int  nReps;
    bool bAllPassed = true;

#if defined(COMPARE_PERFORMANCE)
    // Can choose as many repetitions as you might expect in production.
    nReps = 1000000;
#else
    nReps = 1;
#endif

    while (nReps--)
    {
		// Case with first wildcard after total match.
        bAllPassed &= test("Hi", "Hi*", true);
		
		// Case with mismatch after '*'
        bAllPassed &= test("abc", "ab*d", false);

        // Cases with repeating character sequences.
        bAllPassed &= test("abcccd", "*ccd", true);
        bAllPassed &= test("mississipissippi", "*issip*ss*", true);
        bAllPassed &= test("xxxx*zzzzzzzzy*f", "xxxx*zzy*fffff", false);
        bAllPassed &= test("xxxx*zzzzzzzzy*f", "xxx*zzy*f", true);
        bAllPassed &= test("xxxxzzzzzzzzyf", "xxxx*zzy*fffff", false);
        bAllPassed &= test("xxxxzzzzzzzzyf", "xxxx*zzy*f", true);
        bAllPassed &= test("xyxyxyzyxyz", "xy*z*xyz", true);
        bAllPassed &= test("mississippi", "*sip*", true);
        bAllPassed &= test("xyxyxyxyz", "xy*xyz", true);
        bAllPassed &= test("mississippi", "mi*sip*", true);
        bAllPassed &= test("ababac", "*abac*", true);
        bAllPassed &= test("ababac", "*abac*", true);
        bAllPassed &= test("aaazz", "a*zz*", true);
        bAllPassed &= test("a12b12", "*12*23", false);
        bAllPassed &= test("a12b12", "a12b", false);
        bAllPassed &= test("a12b12", "*12*12*", true);

#if !defined(COMPARE_PERFORMANCE)
		// From DDJ reader Andy Belf: a case of repeating text matching the 
		// different kinds of wildcards in order of '*' and then '?'.
        bAllPassed &= test("caaab", "*a?b", true);
		// This similar case was found, probably independently, by Dogan Kurt.
        bAllPassed &= test("aaaaa", "*aa?", true);
#endif

        // Additional cases where the '*' char appears in the tame string.
        bAllPassed &= test("*", "*", true);
        bAllPassed &= test("a*abab", "a*b", true);
        bAllPassed &= test("a*r", "a*", true);
        bAllPassed &= test("a*ar", "a*aar", false);

        // More double wildcard scenarios.
        bAllPassed &= test("XYXYXYZYXYz", "XY*Z*XYz", true);
        bAllPassed &= test("missisSIPpi", "*SIP*", true);
        bAllPassed &= test("mississipPI", "*issip*PI", true);
        bAllPassed &= test("xyxyxyxyz", "xy*xyz", true);
        bAllPassed &= test("miSsissippi", "mi*sip*", true);
        bAllPassed &= test("miSsissippi", "mi*Sip*", false);
        bAllPassed &= test("abAbac", "*Abac*", true);
        bAllPassed &= test("abAbac", "*Abac*", true);
        bAllPassed &= test("aAazz", "a*zz*", true);
        bAllPassed &= test("A12b12", "*12*23", false);
        bAllPassed &= test("a12B12", "*12*12*", true);
        bAllPassed &= test("oWn", "*oWn*", true);

        // Completely tame (no wildcards) cases.
        bAllPassed &= test("bLah", "bLah", true);
        bAllPassed &= test("bLah", "bLaH", false);

        // Simple mixed wildcard tests suggested by Marlin Deckert.
        bAllPassed &= test("a", "*?", true);
        bAllPassed &= test("ab", "*?", true);
        bAllPassed &= test("abc", "*?", true);

        // More mixed wildcard tests including coverage for false positives.
        bAllPassed &= test("a", "??", false);
        bAllPassed &= test("ab", "?*?", true);
        bAllPassed &= test("ab", "*?*?*", true);
        bAllPassed &= test("abc", "?**?*?", true);
        bAllPassed &= test("abc", "?**?*&?", false);
        bAllPassed &= test("abcd", "?b*??", true);
        bAllPassed &= test("abcd", "?a*??", false);
        bAllPassed &= test("abcd", "?**?c?", true);
        bAllPassed &= test("abcd", "?**?d?", false);
        bAllPassed &= test("abcde", "?*b*?*d*?", true);

        // Single-character-match cases.
        bAllPassed &= test("bLah", "bL?h", true);
        bAllPassed &= test("bLaaa", "bLa?", false);
        bAllPassed &= test("bLah", "bLa?", true);
        bAllPassed &= test("bLaH", "?Lah", false);
        bAllPassed &= test("bLaH", "?LaH", true);

        // Many-wildcard scenarios.
        bAllPassed &= test("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\
aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaab", 
            "a*a*a*a*a*a*aa*aaa*a*a*b", true);
        bAllPassed &= test("abababababababababababababababababababaacacacacaca\
cacadaeafagahaiajakalaaaaaaaaaaaaaaaaaffafagaagggagaaaaaaaab", 
            "*a*b*ba*ca*a*aa*aaa*fa*ga*b*", true);
        bAllPassed &= test("abababababababababababababababababababaacacacacaca\
cacadaeafagahaiajakalaaaaaaaaaaaaaaaaaffafagaagggagaaaaaaaab", 
            "*a*b*ba*ca*a*x*aaa*fa*ga*b*", false);
        bAllPassed &= test("abababababababababababababababababababaacacacacaca\
cacadaeafagahaiajakalaaaaaaaaaaaaaaaaaffafagaagggagaaaaaaaab", 
            "*a*b*ba*ca*aaaa*fa*ga*gggg*b*", false);
        bAllPassed &= test("abababababababababababababababababababaacacacacaca\
cacadaeafagahaiajakalaaaaaaaaaaaaaaaaaffafagaagggagaaaaaaaab", 
            "*a*b*ba*ca*aaaa*fa*ga*ggg*b*", true);
        bAllPassed &= test("aaabbaabbaab", "*aabbaa*a*", true);
        bAllPassed &= test("a*a*a*a*a*a*a*a*a*a*a*a*a*a*a*a*a*", 
            "a*a*a*a*a*a*a*a*a*a*a*a*a*a*a*a*a*", true);
        bAllPassed &= test("aaaaaaaaaaaaaaaaa", 
            "*a*a*a*a*a*a*a*a*a*a*a*a*a*a*a*a*a*", true);
        bAllPassed &= test("aaaaaaaaaaaaaaaa", 
            "*a*a*a*a*a*a*a*a*a*a*a*a*a*a*a*a*a*", false);
        bAllPassed &= test("abc*abcd*abcde*abcdef*abcdefg*abcdefgh*abcdefghi*a\
bcdefghij*abcdefghijk*abcdefghijkl*abcdefghijklm*abcdefghijklmn", 
            "abc*abc*abc*abc*abc*abc*abc*abc*abc*abc*abc*abc*abc*abc*abc*abc*a\
            bc*", false);
        bAllPassed &= test("abc*abcd*abcde*abcdef*abcdefg*abcdefgh*abcdefghi*a\
bcdefghij*abcdefghijk*abcdefghijkl*abcdefghijklm*abcdefghijklmn", 
            "abc*abc*abc*abc*abc*abc*abc*abc*abc*abc*abc*abc*", true);
        bAllPassed &= test("abc*abcd*abcd*abc*abcd", "abc*abc*abc*abc*abc", 
            false);
        bAllPassed &= test(
            "abc*abcd*abcd*abc*abcd*abcd*abc*abcd*abc*abc*abcd", 
            "abc*abc*abc*abc*abc*abc*abc*abc*abc*abc*abcd", true);
        bAllPassed &= test("abc", "********a********b********c********", 
            true);
        bAllPassed &= test("********a********b********c********", "abc", 
            false);
        bAllPassed &= test("abc", "********a********b********b********", 
            false);
        bAllPassed &= test("*abc*", "***a*b*c***", true);

#if defined(COMPARE_CASE_INSENSITIVE)
        bAllPassed &= test("mississippi", "*issip*PI", true);
		bAllPassed &= test("miSsissippi", "mi*Sip*", true);
		bAllPassed &= test("bLah", "bLaH", true);
#endif

        // Tests suggested by other DDJ readers
        bAllPassed &= test("", "?", false);
        bAllPassed &= test("", "*?", false);
        bAllPassed &= test("", "", true);
        bAllPassed &= test("a", "", false);
    }

    if (bAllPassed)
    {
        printf("Passed wild string tests\n");
    }
    else
    {
        printf("Failed wild string tests\n");
    }

    return;
}


// A set of tests with (almost) no '*' wildcards.
//
void testtame(void)
{
    int  nReps;
    bool bAllPassed = true;

#if defined(COMPARE_PERFORMANCE)
    // Can choose as many repetitions as you might expect in production.
    nReps = 1000000;
#else
    nReps = 1;
#endif

    while (nReps--)
    {
		// Case with last character mismatch.
        bAllPassed &= test("abc", "abd", false);

        // Cases with repeating character sequences.
        bAllPassed &= test("abcccd", "abcccd", true);
        bAllPassed &= test("mississipissippi", "mississipissippi", true);
        bAllPassed &= test("xxxxzzzzzzzzyf", "xxxxzzzzzzzzyfffff", false);
        bAllPassed &= test("xxxxzzzzzzzzyf", "xxxxzzzzzzzzyf", true);
        bAllPassed &= test("xxxxzzzzzzzzyf", "xxxxzzy.fffff", false);
        bAllPassed &= test("xxxxzzzzzzzzyf", "xxxxzzzzzzzzyf", true);
        bAllPassed &= test("xyxyxyzyxyz", "xyxyxyzyxyz", true);
        bAllPassed &= test("mississippi", "mississippi", true);
        bAllPassed &= test("xyxyxyxyz", "xyxyxyxyz", true);
        bAllPassed &= test("m ississippi", "m ississippi", true);
        bAllPassed &= test("ababac", "ababac?", false);
        bAllPassed &= test("dababac", "ababac", false);
        bAllPassed &= test("aaazz", "aaazz", true);
        bAllPassed &= test("a12b12", "1212", false);
        bAllPassed &= test("a12b12", "a12b", false);
        bAllPassed &= test("a12b12", "a12b12", true);

        // A mix of cases
        bAllPassed &= test("n", "n", true);
        bAllPassed &= test("aabab", "aabab", true);
        bAllPassed &= test("ar", "ar", true);
        bAllPassed &= test("aar", "aaar", false);
        bAllPassed &= test("XYXYXYZYXYz", "XYXYXYZYXYz", true);
        bAllPassed &= test("missisSIPpi", "missisSIPpi", true);
        bAllPassed &= test("mississipPI", "mississipPI", true);
        bAllPassed &= test("xyxyxyxyz", "xyxyxyxyz", true);
        bAllPassed &= test("miSsissippi", "miSsissippi", true);
        bAllPassed &= test("miSsissippi", "miSsisSippi", false);
        bAllPassed &= test("abAbac", "abAbac", true);
        bAllPassed &= test("abAbac", "abAbac", true);
		
#if defined(COMPARE_CASE_INSENSITIVE)
		bAllPassed &= test("miSsissippi", "miSsisSippi", true);
		bAllPassed &= test("abAbac", "abAbac", true);
		bAllPassed &= test("abAbac", "abAbac", true);
		bAllPassed &= test("bLah", "bLaH", true);
#endif
		
        bAllPassed &= test("aAazz", "aAazz", true);
        bAllPassed &= test("A12b12", "A12b123", false);
        bAllPassed &= test("a12B12", "a12B12", true);
        bAllPassed &= test("oWn", "oWn", true);
        bAllPassed &= test("bLah", "bLah", true);
        bAllPassed &= test("bLah", "bLaH", false);

        // Single '?' cases.
        bAllPassed &= test("a", "a", true);
        bAllPassed &= test("ab", "a?", true);
        bAllPassed &= test("abc", "ab?", true);

        // Mixed '?' cases.
        bAllPassed &= test("a", "??", false);
        bAllPassed &= test("ab", "??", true);
        bAllPassed &= test("abc", "???", true);
        bAllPassed &= test("abcd", "????", true);
        bAllPassed &= test("abc", "????", false);
        bAllPassed &= test("abcd", "?b??", true);
        bAllPassed &= test("abcd", "?a??", false);
        bAllPassed &= test("abcd", "??c?", true);
        bAllPassed &= test("abcd", "??d?", false);
        bAllPassed &= test("abcde", "?b?d*?", true);

        // Longer string scenarios.
        bAllPassed &= test("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\
aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaab", 
            "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\
aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaab", true);
        bAllPassed &= test("abababababababababababababababababababaacacacacaca\
cacadaeafagahaiajakalaaaaaaaaaaaaaaaaaffafagaagggagaaaaaaaab", 
            "abababababababababababababababababababaacacacacaca\
cacadaeafagahaiajakalaaaaaaaaaaaaaaaaaffafagaagggagaaaaaaaab", true);
        bAllPassed &= test("abababababababababababababababababababaacacacacaca\
cacadaeafagahaiajakalaaaaaaaaaaaaaaaaaffafagaagggagaaaaaaaab", 
            "abababababababababababababababababababaacacacacaca\
cacadaeafagahaiajaxalaaaaaaaaaaaaaaaaaffafagaagggagaaaaaaaab", false);
        bAllPassed &= test("abababababababababababababababababababaacacacacaca\
cacadaeafagahaiajakalaaaaaaaaaaaaaaaaaffafagaagggagaaaaaaaab", 
            "abababababababababababababababababababaacacacacaca\
cacadaeafagahaiajakalaaaaaaaaaaaaaaaaaffafagaggggagaaaaaaaab", false);
        bAllPassed &= test("abababababababababababababababababababaacacacacaca\
cacadaeafagahaiajakalaaaaaaaaaaaaaaaaaffafagaagggagaaaaaaaab", 
            "abababababababababababababababababababaacacacacaca\
cacadaeafagahaiajakalaaaaaaaaaaaaaaaaaffafagaagggagaaaaaaaab", true);
        bAllPassed &= test("aaabbaabbaab", "aaabbaabbaab", true);
        bAllPassed &= test("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa", 
            "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa", true);
        bAllPassed &= test("aaaaaaaaaaaaaaaaa", 
            "aaaaaaaaaaaaaaaaa", true);
        bAllPassed &= test("aaaaaaaaaaaaaaaa", 
            "aaaaaaaaaaaaaaaaa", false);
        bAllPassed &= test("abcabcdabcdeabcdefabcdefgabcdefghabcdefghia\
bcdefghijabcdefghijkabcdefghijklabcdefghijklmabcdefghijklmn", 
            "abcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabc", 
			false);
        bAllPassed &= test("abcabcdabcdeabcdefabcdefgabcdefghabcdefghia\
bcdefghijabcdefghijkabcdefghijklabcdefghijklmabcdefghijklmn", 
            "abcabcdabcdeabcdefabcdefgabcdefghabcdefghia\
bcdefghijabcdefghijkabcdefghijklabcdefghijklmabcdefghijklmn", 
			true);
        bAllPassed &= test("abcabcdabcdabcabcd", "abcabc?abcabcabc", 
            false);
        bAllPassed &= test(
            "abcabcdabcdabcabcdabcdabcabcdabcabcabcd", 
            "abcabc?abc?abcabc?abc?abc?bc?abc?bc?bcd", true);
        bAllPassed &= test("?abc?", "?abc?", true);
    }

    if (bAllPassed)
    {
        printf("Passed tame string tests\n");
    }
    else
    {
        printf("Failed tame string tests\n");
    }

    return;
}


// A set of tests with empty input.
//
void testempty(void)
{
    int  nReps;
    bool bAllPassed = true;

#if defined(COMPARE_PERFORMANCE)
    // Can choose as many repetitions as you might expect in production.
    nReps = 1000000;
#else
    nReps = 1;
#endif

    while (nReps--)
    {
		// A simple case
        bAllPassed &= test("", "abd", false);

        // Cases with repeating character sequences
        bAllPassed &= test("", "abcccd", false);
        bAllPassed &= test("", "mississipissippi", false);
        bAllPassed &= test("", "xxxxzzzzzzzzyfffff", false);
        bAllPassed &= test("", "xxxxzzzzzzzzyf", false);
        bAllPassed &= test("", "xxxxzzy.fffff", false);
        bAllPassed &= test("", "xxxxzzzzzzzzyf", false);
        bAllPassed &= test("", "xyxyxyzyxyz", false);
        bAllPassed &= test("", "mississippi", false);
        bAllPassed &= test("", "xyxyxyxyz", false);
        bAllPassed &= test("", "m ississippi", false);
        bAllPassed &= test("", "ababac*", false);
        bAllPassed &= test("", "ababac", false);
        bAllPassed &= test("", "aaazz", false);
        bAllPassed &= test("", "1212", false);
        bAllPassed &= test("", "a12b", false);
        bAllPassed &= test("", "a12b12", false);

        // A mix of cases
        bAllPassed &= test("", "n", false);
        bAllPassed &= test("", "aabab", false);
        bAllPassed &= test("", "ar", false);
        bAllPassed &= test("", "aaar", false);
        bAllPassed &= test("", "XYXYXYZYXYz", false);
        bAllPassed &= test("", "missisSIPpi", false);
        bAllPassed &= test("", "mississipPI", false);
        bAllPassed &= test("", "xyxyxyxyz", false);
        bAllPassed &= test("", "miSsissippi", false);
        bAllPassed &= test("", "miSsisSippi", false);
        bAllPassed &= test("", "abAbac", false);
        bAllPassed &= test("", "abAbac", false);
        bAllPassed &= test("", "aAazz", false);
        bAllPassed &= test("", "A12b123", false);
        bAllPassed &= test("", "a12B12", false);
        bAllPassed &= test("", "oWn", false);
        bAllPassed &= test("", "bLah", false);
        bAllPassed &= test("", "bLaH", false);

		// Both strings empty
        bAllPassed &= test("", "", true);

		// Another simple case
        bAllPassed &= test("abc", "", false);

        // Cases with repeating character sequences.
        bAllPassed &= test("abcccd", "", false);
        bAllPassed &= test("mississipissippi", "", false);
        bAllPassed &= test("xxxxzzzzzzzzyf", "", false);
        bAllPassed &= test("xxxxzzzzzzzzyf", "", false);
        bAllPassed &= test("xxxxzzzzzzzzyf", "", false);
        bAllPassed &= test("xxxxzzzzzzzzyf", "", false);
        bAllPassed &= test("xyxyxyzyxyz", "", false);
        bAllPassed &= test("mississippi", "", false);
        bAllPassed &= test("xyxyxyxyz", "", false);
        bAllPassed &= test("m ississippi", "", false);
        bAllPassed &= test("ababac", "", false);
        bAllPassed &= test("dababac", "", false);
        bAllPassed &= test("aaazz", "", false);
        bAllPassed &= test("a12b12", "", false);
        bAllPassed &= test("a12b12", "", false);
        bAllPassed &= test("a12b12", "", false);

        // A mix of cases
        bAllPassed &= test("n", "", false);
        bAllPassed &= test("aabab", "", false);
        bAllPassed &= test("ar", "", false);
        bAllPassed &= test("aar", "", false);
        bAllPassed &= test("XYXYXYZYXYz", "", false);
        bAllPassed &= test("missisSIPpi", "", false);
        bAllPassed &= test("mississipPI", "", false);
        bAllPassed &= test("xyxyxyxyz", "", false);
        bAllPassed &= test("miSsissippi", "", false);
        bAllPassed &= test("miSsissippi", "", false);
        bAllPassed &= test("abAbac", "", false);
        bAllPassed &= test("abAbac", "", false);
        bAllPassed &= test("aAazz", "", false);
        bAllPassed &= test("A12b12", "", false);
        bAllPassed &= test("a12B12", "", false);
        bAllPassed &= test("oWn", "", false);
        bAllPassed &= test("bLah", "", false);
        bAllPassed &= test("bLah", "", false);
    }

    if (bAllPassed)
    {
        printf("Passed empty string tests\n");
    }
    else
    {
        printf("Failed empty string tests\n");
    }

    return;
}


// Correctness tests for a case-sensitive arrangement for invoking a 
// UTF-8-enabled routine for matching wildcards.
//
void testutf8(void)
{
    bool bAllPassed = true;

	// Simple correctness tests involving various UTF-8 symbols and 
	// international content.
    bAllPassed &= test("🐂🚀♥🍀貔貅🦁★□√🚦€¥☯🐴😊🍓🐕🎺🧊☀☂🐉", 
	                     "*☂🐉", true);
						 
#if defined(COMPARE_CASE_INSENSITIVE)
		bAllPassed &= test("AbCD", "abc?", true);
		bAllPassed &= test("AbC★", "abc?", true);
        bAllPassed &= test("⚛⚖☁o", "⚛⚖☁O", true);
#endif
		
	bAllPassed &= test("▲●🐎✗🤣🐶♫🌻ॐ", "▲●☂*", false);
	bAllPassed &= test("𓋍𓋔𓎍", "𓋍𓋔?", true);
	bAllPassed &= test("𓋍𓋔𓎍", "𓋍?𓋔𓎍", false);
	bAllPassed &= test("♅☌♇", "♅☌♇", true);
	bAllPassed &= test("⚛⚖☁", "⚛🍄☁", false);
	bAllPassed &= test("⚛⚖☁O", "⚛⚖☁0", false);
	bAllPassed &= test("गते गते पारगते पारसंगते बोधि स्वाहा", 
	                     "गते गते पारगते प????गते बोधि स्वाहा", true);
	bAllPassed &= test(
	    "Мне нужно выучить русский язык, чтобы лучше оценить Пушкина.", 
	    "Мне нужно выучить * язык, чтобы лучше оценить *.", true);
	bAllPassed &= test(
	    "אני צריך ללמוד אנגלית כדי להעריך את גינסברג", 
	    " אני צריך ללמוד אנגלית כדי להעריך את ???????", false);
	bAllPassed &= test(
	    "ગિન્સબર્ગની શ્રેષ્ઠ પ્રશંસા કરવા માટે મારે અંગ્રેજી શીખવું પડશે.", 
	    "* શ્રેષ્ઠ પ્રશંસા કરવા માટે મારે * શીખવું પડશે.", true);
	bAllPassed &= test(
	    "ગિન્સબર્ગની શ્રેષ્ઠ પ્રશંસા કરવા માટે મારે અંગ્રેજી શીખવું પડશે.", 
	    "??????????? શ્રેષ્ઠ પ્રશંસા કરવા માટે મારે * શીખવું પડશે.", true);
	bAllPassed &= test(
	    "ગિન્સબર્ગની શ્રેષ્ઠ પ્રશંસા કરવા માટે મારે અંગ્રેજી શીખવું પડશે.", 
	    "ગિન્સબર્ગની શ્રેષ્ઠ પ્રશંસા કરવા માટે મારે હિબ્રુ ભાષા શીખવી પડશે.", false);
	
	// These tests involve multiple=byte code points that contain bytes 
	// identical to the single-byte code points for '*' and '?'.
	bAllPassed &= test("ḪؿꜪἪꜿ", "ḪؿꜪἪꜿ", true);
	bAllPassed &= test("ḪؿUἪꜿ", "ḪؿꜪἪꜿ", false);
	bAllPassed &= test("ḪؿꜪἪꜿ", "ḪؿꜪἪꜿЖ", false);
	bAllPassed &= test("ḪؿꜪἪꜿ", "ЬḪؿꜪἪꜿ", false);
	bAllPassed &= test("ḪؿꜪἪꜿ", "?ؿꜪ*ꜿ", true);

    if (bAllPassed)
    {
        printf("Passed UTF-8 tests\n");
    }
    else
    {
        printf("Failed UTF-8 tests\n");
    }
	
	return;
}


int main(void)
{
#if defined(COMPARE_PERFORMANCE)
	// Clear accumulated times and the UTF-8 test status flag.
	iAccumulatedTimeUtf8 = iAccumulatedTimeLenUtf8 = iAccumulatedTimeAscii = 0;
#endif

#if defined(COMPARE_TAME)
	testtame();
#endif

#if defined(COMPARE_EMPTY)
	testempty();
#endif

#if defined(COMPARE_WILD)
	testwild();
#endif

#if defined(COMPARE_UTF8)
	testutf8();
#endif

#if defined(COMPARE_PERFORMANCE)
    // Timings have been accumulated via file-scope data.
    double fBase = 10.0;
    double fExpNanoseconds = 9.0;
    double fExpMilliseconds = 3.0;

    // Represent the timings in seconds, to millisecond precision.
    double fTimeCumulativeAsciiVersion = 
            ((double) (iAccumulatedTimeAscii) /
                pow(fBase, fExpNanoseconds)) * pow(fBase, fExpMilliseconds);
    double fTimeCumulativeUtf8Version = 
            ((double) (iAccumulatedTimeUtf8) /
                pow(fBase, fExpNanoseconds)) * pow(fBase, fExpMilliseconds);
    double fTimeCumulativeLenUtf8Version = 
            ((double) (iAccumulatedTimeLenUtf8) /
                pow(fBase, fExpNanoseconds)) * pow(fBase, fExpMilliseconds);
    // Can set up similar calculations for more performance comparisons.

    float fAsciiVersionTimeInSeconds = fTimeCumulativeAsciiVersion / 1000;
    float fUtf8VersionTimeInSeconds = fTimeCumulativeUtf8Version / 1000;
    float fUtf8LenVersionTimeInSeconds = fTimeCumulativeLenUtf8Version / 1000;

    // Show the timing results.
    printf(
       "FastWildCompare() - for ASCII strings: %.3f seconds\n",
           fAsciiVersionTimeInSeconds);
    printf(
       "FastWildCompareUtf8() - for UTF-8-encoded strings: %.3f seconds\n",
           fUtf8VersionTimeInSeconds);
    printf(
       "FastWildLenCompareUtf8() - for UTF-8-encoded strings: %.3f seconds\n",
           fUtf8LenVersionTimeInSeconds);
#endif

	return 0;

}
