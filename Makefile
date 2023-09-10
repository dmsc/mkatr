#
#  Copyright (C) 2016 Daniel Serpell
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 2 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License along
#  with this program.  If not, see <http://www.gnu.org/licenses/>
#

PROGS=\
 mkatr\
 lsatr\

SOURCES_mkatr=\
 crc32.c\
 darray.c\
 flist.c\
 mkatr.c\
 msg.c\
 spartafs.c\

SOURCES_lsatr=\
 atr.c\
 lsatr.c\
 lssfs.c\
 msg.c\

CFLAGS=-O2 -Wall
LDFLAGS=

BUILD_DIR=obj

# Default rule
all: $(PROGS)

# Rule template
define PROG_template
 # Objects from sources
 OBJS_$(1)=$(addprefix $(BUILD_DIR)/,$(SOURCES_$(1):%.c=%.o))
 # All SOURCES/OBJECTS
 SOURCES+=$$(SOURCES_$(1))
 OBJS+=$$(OBJS_$(1))
 # Link rule
$(1): $$(OBJS_$(1))
	$$(CC) $$(CFLAGS) $$(LDFLAGS) $$^ $$(LDLIBS) -o $$@
endef

# Generate all rules
$(foreach prog,$(PROGS),$(eval $(call PROG_template,$(prog))))

DEPS=$(OBJS:%.o=%.d)

# Cleanup
.PHONY: clean
clean:
	-rm -f $(OBJS) $(DEPS)
	-rmdir $(BUILD_DIR)

.PHONY: distclean
distclean: clean
	-rm -f $(PROGS)

# Create output dirs
$(BUILD_DIR):
	mkdir -p $@

$(OBJS): | $(BUILD_DIR)
$(DEPS): | $(BUILD_DIR)

# Compilation
$(BUILD_DIR)/%.o: src/%.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $@ $<

# Dependencies
$(BUILD_DIR)/%.d: src/%.c
	@$(CC) -MM -MP -MF $@ -MT "$(@:.d=.o) $@" $(CFLAGS) $(CPPFLAGS) $<

ifneq "$(MAKECMDGOALS)" "clean"
 ifneq "$(MAKECMDGOALS)" "distclean"
  -include $(DEPS)
 endif
endif
