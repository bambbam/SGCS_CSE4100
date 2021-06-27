[system programming lecture]

-project 4 Phase3

csapp.{c,h}
        CS:APP3e functions

myshell.{c,h}
        시그널 핸들링이 처리된 myshell

백그라운드 프로세스를 처리하고, jobs를 처리한다.

foreground 프로세스와 background 프로세스를 따로 처리한다.

처리하는 명령어 예시: 
ls -al | grep myshell & : 기술 된 명령어를 background 프로세스로 돌린다.
python interpreter 실행 이후 ctrl z : 실행되고 있는 foreground 명령어를 suspend상태로 만들어서 joblist에 올린다.
sleep 10 실행 이후 ctrl c : 실행되고 있는 foreground 명령어를 강제 종료한다.
fg %1 : 1번 jid를 가진 suspend 또는 background 프로세스를 foreground로 올린다.
bg %1 : suspend되어있는 프로세스를 background로 돌린다.
kill %1 : 1번 jid를 가진 프로세스를 강제종료한다.
