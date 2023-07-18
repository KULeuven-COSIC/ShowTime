THIS_DIR := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))
UTILS_DIR := ../utils

##################
# Build directory
##################
OBJDIR  = obj
CFLAGS += -I./$(OBJDIR)
CFLAGS += -I$(EVSET_DIR)/$(OBJDIR)
CFLAGS += -I$(UTILS_DIR)/$(OBJDIR)
CFLAGS += -Wall -Wno-unused-variable

####################
# Files and folders
####################

SRCS_TMR = 	main.c timer_free.c attacker_helper.c

OBJS_TMR = 	$(addprefix $(OBJDIR)/,$(patsubst %.c,%.o,$(SRCS_TMR)))
OBJS_UTL = 	$(addprefix $(UTILS_DIR)/$(OBJDIR)/,$(notdir $(patsubst %.c,%.o,$(shell find $(UTILS_DIR)/*.c))))

##########
# Targets
##########
all: clean utils arch_reorder

# Execution targets
run:
	./arch_reorder

# Compiling targets

arch_reorder: $(OBJS_TMR) $(OBJS_UTL)
	$(CC) -o $@ $^ $(LDFLAGS) -lrt -lm

utils: .PHONY
	$(MAKE) -C $(UTILS_DIR)

$(OBJDIR)/%.o: %.c | objdir
	$(CC) $(CFLAGS) -c $< -o $@

objdir:
	@mkdir -p $(OBJDIR)

# Cleaning targets

clean:
	rm -rf arch_reorder
	rm -rf $(OBJDIR) log/*
	$(MAKE) clean -C $(UTILS_DIR)/

.PHONY: clean
