~~~ compiler.c        ~~~
~~~ Maksim Petrushin  ~~~
~~~ Compiler for PL/0 ~~~
~~~ 11/21/2023        ~~~


1. Compile the C file:
    gcc compiler.c

2. Run it:
    ./a.out input.txt 

    ~~ It takes 1 command line argument (EX: input.txt) ~~
    ~~ It it prints to the console (stdout), and creates ~~
    ~~ an "elf.txt" file with purely VM numberical code" ~~

3. Output prints to console + elf.txt is generated

    ~~ This "elf.txt" is what is then used as a command ~~
    ~~ line argument into my VM (HW1). Note there is no ~~
    ~~ ODD instruction. At the time it wasn't reqired. ~~

4. NOTE! (IMPORTANT requirement): 

    ~~ Your input PL0 source code, cannot start the with ~~
    ~~ the leading whitespace. i.e. the very first PL0 code ~~
    ~~ character must be the start of your code, and NOT a ~~
    ~~ stream of whitespaces ~~

5. Unit Test test cases:
    ./test.sh
    
    ~~ If doesnt run, adjust shebang (top comment) ~~
    ~~ if needed by typing "which bash" in terminal ~~
    ~~ and placing it's output on the first line ~~
    ~~ instead of /bin/bash. Also give yourself the ~~
    ~~ executable permissions by typing the command ~~
    ~~ "chmod +x test.sh". The scripts execusion is ~~
    ~~ to automatically compile my C source file and ~~
    ~~ to run through my extra 67 .txt test cases ~~
    ~~ All of them must be in the same directory ~~
    ~~ Format of .txt's: inX.txt (input), baseX.txt ~~
    ~~ (what correct output must normally be), and ~~
    ~~ (outX.txt what the actual output is), where X ~~
    ~~ is the number of the test case (ex: 1, .. ,67)~~
    ~~ If all my extra test cases passed, my scripting~~
    ~~ program will output to console the following: ~~

    
Compile of compiler.c succeded.

-> Case #1 - in1.txt == base1.txt
-> Case #2 - in2.txt == base2.txt
-> Case #3 - in3.txt == base3.txt
-> Case #4 - in4.txt == base4.txt
-> Case #5 - in5.txt == base5.txt
-> Case #6 - in6.txt == base6.txt
-> Case #7 - in7.txt == base7.txt
-> Case #8 - in8.txt == base8.txt
-> Case #9 - in9.txt == base9.txt
-> Case #10 - in10.txt == base10.txt
-> Case #11 - in11.txt == base11.txt
-> Case #12 - in12.txt == base12.txt
-> Case #13 - in13.txt == base13.txt
-> Case #14 - in14.txt == base14.txt
-> Case #15 - in15.txt == base15.txt
-> Case #16 - in16.txt == base16.txt
-> Case #17 - in17.txt == base17.txt
-> Case #18 - in18.txt == base18.txt
-> Case #19 - in19.txt == base19.txt
-> Case #20 - in20.txt == base20.txt
-> Case #21 - in21.txt == base21.txt
-> Case #22 - in22.txt == base22.txt
-> Case #23 - in23.txt == base23.txt
-> Case #24 - in24.txt == base24.txt
-> Case #25 - in25.txt == base25.txt
-> Case #26 - in26.txt == base26.txt
-> Case #27 - in27.txt == base27.txt
-> Case #28 - in28.txt == base28.txt
-> Case #29 - in29.txt == base29.txt
-> Case #30 - in30.txt == base30.txt
-> Case #31 - in31.txt == base31.txt
-> Case #32 - in32.txt == base32.txt
-> Case #33 - in33.txt == base33.txt
-> Case #34 - in34.txt == base34.txt
-> Case #35 - in35.txt == base35.txt
-> Case #36 - in36.txt == base36.txt
-> Case #37 - in37.txt == base37.txt
-> Case #38 - in38.txt == base38.txt
-> Case #39 - in39.txt == base39.txt
-> Case #40 - in40.txt == base40.txt
-> Case #41 - in41.txt == base41.txt
-> Case #42 - in42.txt == base42.txt
-> Case #43 - in43.txt == base43.txt
-> Case #44 - in44.txt == base44.txt
-> Case #45 - in45.txt == base45.txt
-> Case #46 - in46.txt == base46.txt
-> Case #47 - in47.txt == base47.txt
-> Case #48 - in48.txt == base48.txt
-> Case #49 - in49.txt == base49.txt
-> Case #50 - in50.txt == base50.txt
-> Case #51 - in51.txt == base51.txt
-> Case #52 - in52.txt == base52.txt
-> Case #53 - in53.txt == base53.txt
-> Case #54 - in54.txt == base54.txt
-> Case #55 - in55.txt == base55.txt
-> Case #56 - in56.txt == base56.txt
-> Case #57 - in57.txt == base57.txt
-> Case #58 - in58.txt == base58.txt
-> Case #59 - in59.txt == base59.txt
-> Case #60 - in60.txt == base60.txt
-> Case #61 - in61.txt == base61.txt
-> Case #62 - in62.txt == base62.txt
-> Case #63 - in63.txt == base63.txt
-> Case #64 - in64.txt == base64.txt
-> Case #65 - in65.txt == base65.txt
-> Case #66 - in66.txt == base66.txt
-> Case #67 - in67.txt == base67.txt

Compiler testing completed!

