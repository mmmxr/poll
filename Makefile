
.PHONY:all
poll:poll.c
	gcc -o $@ $^


.PHONY:clean
clean:
	rm -f poll
