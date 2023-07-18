THIS_DIR := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))
EVSET_DIR := ../evsets
UTILS_DIR := ../utils

##################
# Build directory
##################
OBJDIR  = obj
LOGDIR  = log
CFLAGS += -I./$(OBJDIR) 
CFLAGS += -I$(EVSET_DIR)/$(OBJDIR)
CFLAGS += -I$(UTILS_DIR)/$(OBJDIR)
CFLAGS += -Wall -Wno-unused-variable

####################
# Files and folders
####################
SRCS_AMP = 	ampL1fy.c

OBJS_AMP 	= 	$(addprefix $(OBJDIR)/,$(patsubst %.c,%.o,$(SRCS_AMP)))
OBJS_EVS 	= 	$(addprefix $(EVSET_DIR)/$(OBJDIR)/,$(notdir $(patsubst %.c,%.o,$(shell find $(EVSET_DIR)/*.c $(EVSET_DIR)/list/*.c))))
OBJS_UTL	= 	$(addprefix $(UTILS_DIR)/$(OBJDIR)/,$(notdir $(patsubst %.c,%.o,$(shell find $(UTILS_DIR)/*.c))))

##########
# Targets
##########
all: clean utils evsets amp 

# Compiling targets

amp: $(OBJS_AMP) $(OBJS_EVS) $(OBJS_UTL)
	$(CC) -o $@ $^ $(LDFLAGS) -lrt -lm

utils:
	$(MAKE) -C $(UTILS_DIR) 

evsets:
	$(MAKE) -C $(EVSET_DIR) CONFIG_FILE=$(UTILS_DIR)/configuration.h

$(OBJDIR)/%.o: %.c | objdir
	$(CC) $(CFLAGS) -c $< -o $@

objdir:
	@mkdir -p $(OBJDIR) 
	@mkdir -p $(LOGDIR) 

# Execution targets

run: 
	rm -rf $(LOGDIR)/*.log
	@./amp
	@make histo

histo:
	@python3 histo.py

# Cleaning targets

clean:
	rm -rf amp 
	rm -rf $(OBJDIR) 
	$(MAKE) clean -C $(EVSET_DIR)/
	$(MAKE) clean -C $(UTILS_DIR)/

.PHONY: all clean
