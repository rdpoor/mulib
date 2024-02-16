Given the string:

```
  {"a":10, "b":-11 , "c":[3.2, 10e5], "d":[]}

ch  ch_clas states               i top token  notes
' ' C_SPACE GO => GO             0 -
' ' C_SPACE GO => GO             0 -
'{' C_LCURB GO => OPEN-{ => OB   1 [OBJECT 0]   start {   // why not => KE?
'"' C_QUOTE OB => ST             2 [STRING 1]   start "
'a' C_LOW_A ST => ST             2 [STRING 1]
'"' C_QUOTE ST => QUOTE  => CO   2 [STRING 1]   finish "a"
':' C_COLON CO => COLON  => VA   2 [STRING 1]
'1' C_DIGIT VA => IN             3 [INTEGER 1]  start integer
'0' C_ZERO IN => IN              3 [INTEGER 1]
',' C_COMMA IN => COMMA  => KE   3 [INTEGER 1]  end integer
' ' C_SPACE KE => KE             3 [INTEGER 1]
'"' C_QUOTE KE => ST             4 [STRING 1]   start "
'b' C_LOW_B ST => ST             4 [STRING 1]
'"' C_QUOTE ST => QUOTE  => CO   4 [STRING 1]   finish "b"
':' C_COLON CO => COLON  => VA   4 [STRING 1]
'-' C_MINUS VA => MI             5 [INTEGER 1]  start integer
'1' C_DIGIT MI => IN             5 [INTEGER 1]
'1' C_DIGIT IN => IN             5 [INTEGER 1]
' ' C_SPACE IN => OK             5 [INTEGER 1]  finish -11
',' C_COMMA OK => COMMA  => KE   5 [INTEGER 1]
' ' C_SPACE KE => KE             5 [INTEGER 1]
'"' C_QUOTE KE => ST             6 [STRING 1]   start "
'c' C_LOW_C ST => ST             6 [STRING 1]
'"' C_QUOTE ST => QUOTE  => CO   6 [STRING 1]   finish "c"
':' C_COLON CO => COLON  => VA   6 [STRING 1]
'[' C_LSQRB VA => OPEN=[ => AR   7 [ARRAY 1]    start [
'3' C_DIGIT AR => IN             8 [INTEGER 2]  start integer
'.' C_POINT IN => FR             8 [NUMBER 2]   convert integer to number
'2' C_DIGIT FR => FS             8 [NUMBER 2]
',' C_COMMA FS => COMMA  => VA   8 [NUMBER 2]   finish 3.2
' ' C_SPACE VA => VA             8 [NUMBER 2]
'1' C_DIGIT VA => IN             9 [INTEGER 2]  start integer
'0' C_ZERO IN => IN              9 [INTEGER 2]
'e' C_LOW_E IN => E1             9 [NUMBER 2]   convert nuber to number
'5' C_DIGIT E1 => E3             9 [INTEGER 2]
']' C_RSQRB E3 => CLOSE-]=> OK   9 [INTEGER 2]  finish 10e5, finish [3.2, 10e5]
',' C_COMMA OK => COMMA  => KE   9 [INTEGER 2]
' ' C_SPACE KE => KE             9 [INTEGER 2]
'"' C_QUOTE KE => ST            10 [STRING 1]   start "
'd' C_LOW_D ST => ST            10 [STRING 1]
'"' C_QUOTE ST => QUOTE  => CO  10 [STRING 1]   finish "d"
':' C_COLON CO => COLON  => VA  10 [STRING 1]
'[' C_LSQRB VA => OPEN=[ => AR  11 [ARRAY 1]    start [
']' C_RSQRB AR => CLOSE-]=> OK  11 [ARRAY 1]    finish []
'}' C_RCURB OK => CLOSE-}=> OK  11 [ARRAY 1]    finish {"a":10, "b":-11 , "c":[3.2, 10e5], "d":[]}
'\n'C_WHITE OK => OK            11 [ARRAY 1]    mark last element as is_last
```

