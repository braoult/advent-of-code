       IDENTIFICATION DIVISION.
       PROGRAM-ID. AoC-2020-day-1-part-1.
*>         Inspired by https://github.com/GaloisGirl/Coding
*>         Compilation: cobc -F -fmfcomment -std=rm -x ex2-cob.cob
*>         Utilisation: ./ex2-cob < INPUT.txt
       AUTHOR. Bruno Raoult.

       ENVIRONMENT DIVISION.
       INPUT-OUTPUT SECTION.
       FILE-CONTROL.
           SELECT INPUTFILE ASSIGN TO KEYBOARD
               ORGANIZATION IS LINE SEQUENTIAL.

       DATA DIVISION.
       FILE SECTION.
       FD INPUTFILE.
       01 INPUTRECORD PIC 9(4).

       WORKING-STORAGE SECTION.
       01 FILE-STATUS PIC 9 VALUE 0.
       01 LEN PIC 9(3) VALUE 0.
       01 ARRAY.
           02 ARR OCCURS 0 TO 1024 DEPENDING ON LEN.
               05 VAL PIC 9(5).
       01 S PIC 9(4).
       01 P PIC 9(12).
       01 FMT PIC Z(12)9.

       LOCAL-STORAGE SECTION.
       01 I USAGE UNSIGNED-INT VALUE 1.
       01 J USAGE UNSIGNED-INT VALUE 1.
       01 K USAGE UNSIGNED-INT VALUE 1.
       01 TMP USAGE UNSIGNED-INT VALUE 1.

       PROCEDURE DIVISION.
       01-MAIN.
           OPEN INPUT INPUTFILE.
           PERFORM 02-READ UNTIL FILE-STATUS = 1.
           CLOSE INPUTFILE.
*>           PERFORM 06-PRINT.
           SORT ARR ASCENDING KEY VAL
*>           DISPLAY "=========================".
*>           PERFORM 06-PRINT.
           PERFORM 04-LOOP
           STOP RUN.
       02-READ.
           READ INPUTFILE
               AT END MOVE 1 TO FILE-STATUS
               NOT AT END PERFORM 03-WRITE-TO-TABLE
           END-READ.

       03-WRITE-TO-TABLE.
           ADD 1 TO LEN.
           UNSTRING INPUTRECORD INTO VAL(LEN).
*> Wrong: "234" becomes 2340 instead of 0234
*>           COMPUTE VAL(LEN) = INPUTRECORD.
*>           DISPLAY VAL(LEN) " " LEN.
       04-LOOP.
           PERFORM VARYING I FROM 1 BY 1 UNTIL I > LEN
               ADD I 1 GIVING TMP
               PERFORM VARYING J FROM TMP BY 1 UNTIL J > LEN
                   ADD VAL(I) VAL(J) GIVING S
*>                     DISPLAY I J K
                       IF S = 2020 THEN
                           MULTIPLY VAL(I) BY VAL(J) GIVING P
                           MOVE P TO FMT
                           DISPLAY FMT
                           EXIT PERFORM
                       ELSE
                           IF S > 2020 THEN
                               EXIT PERFORM
                           END-IF
                       END-IF
               END-PERFORM
           END-PERFORM.
       06-PRINT.
           PERFORM VARYING I FROM 1 BY 1 UNTIL I > LEN
               DISPLAY VAL(I)
           END-PERFORM.
