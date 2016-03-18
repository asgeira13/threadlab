
TEAM = $(shell egrep '^ Group:' students.txt | sed -e 's/ Group: *//g' | sed 's/ *\([^ ].*\) *$$/\1/g')
USER_1 = $(shell egrep '^ User 1:' students.txt | sed 's/ User 1: *//g' | sed 's/ *\([^ ].*\) *$$/\1/g')
USER_2 = $(shell egrep '^ User 2:' students.txt | sed 's/ User 2: *//g' | sed 's/ *\([^ ].*\) *$$/\1/g')

VERSION = 1
HANDINDIR = /labs/sty16/.handin/threadlab

handin: 
	@echo "Team: \"$(TEAM)\""
	@echo "User 1: \"$(USER_1)\""
	@echo "User 2: \"$(USER_2)\""
	@if [ "$(TEAM)" == "" ]; then echo "Team name missing, please add it to the students.txt file."; exit 1; fi
	@if [ "$(USER_1)" != "" ]; then getent passwd $(USER_1) > /dev/null; if [ $$? -ne 0 ]; then echo "User $(USER_1) does not exist on Skel."; exit 2; fi; fi
	@if [ "$(USER_2)" != "" ]; then getent passwd $(USER_2) > /dev/null; if [ $$? -ne 0 ]; then echo "User $(USER_2) does not exist on Skel."; exit 3; fi; fi
	cp students.txt "$(HANDINDIR)/$(USER)/$(TEAM)-$(VERSION)-students.txt"
	@chmod 600 "$(HANDINDIR)/$(USER)/$(TEAM)-$(VERSION)-students.txt"
	cp a/traffic.c "$(HANDINDIR)/$(USER)/$(TEAM)-$(VERSION)-a-traffic.c"
	@chmod 600 "$(HANDINDIR)/$(USER)/$(TEAM)-$(VERSION)-a-traffic.c"
	cp b/traffic.c "$(HANDINDIR)/$(USER)/$(TEAM)-$(VERSION)-b-traffic.c"
	@chmod 600 "$(HANDINDIR)/$(USER)/$(TEAM)-$(VERSION)-b-traffic.c"
	@echo
	@echo "Handin successfull"

check:
	ls -lR "$(HANDINDIR)/$(USER)/"
