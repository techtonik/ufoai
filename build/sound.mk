###################################################################################################
# SDL
###################################################################################################

SND_SDL_SRCS=ports/unix/unix_snd_sdl.c
SND_SDL_OBJS=$(SND_SDL_SRCS:%.c=$(BUILDDIR)/snd-sdl/%.o)
SND_SDL_TARGET=snd_sdl.$(SHARED_EXT)

ifeq ($(BUILD_CLIENT), 1)
ifeq ($(HAVE_SND_SDL),1)
	TARGETS += $(SND_SDL_TARGET)
	ALL_OBJS += $(SND_SDL_OBJS)
endif
endif

# Say about to build the target
$(SND_SDL_TARGET) : $(SND_SDL_OBJS) $(BUILDDIR)/.dirs
	@echo " * [SDL] ... linking $(LNKFLAGS) ($(SND_SDL_LIBS))"; \
		$(CC) $(LDFLAGS) $(SHARED_LDFLAGS) -o $@ $(SND_SDL_OBJS) $(SND_LIBS_LIBS) $(LNKFLAGS)

# Say how to build .o files from .c files for this module
$(BUILDDIR)/snd-sdl/%.o: $(SRCDIR)/%.c $(BUILDDIR)/.dirs
	@echo " * [SDL] $<"; \
		$(CC) $(CFLAGS) $(CPPFLAGS) $(SHARED_CFLAGS) $(SDL_CFLAGS) -o $@ -c $< -MD -MT $@ -MP

###################################################################################################
# ALSA
###################################################################################################

SND_ALSA_SRCS=ports/linux/linux_snd_alsa.c
SND_ALSA_OBJS=$(SND_ALSA_SRCS:%.c=$(BUILDDIR)/snd-alsa/%.o)
SND_ALSA_TARGET=snd_alsa.$(SHARED_EXT)

ifeq ($(BUILD_CLIENT), 1)
ifeq ($(HAVE_SND_ALSA),1)
	TARGETS += $(SND_ALSA_TARGET)
	ALL_OBJS += $(SND_ALSA_OBJS)
endif
endif

# Say about to build the target
$(SND_ALSA_TARGET) : $(SND_ALSA_OBJS) $(BUILDDIR)/.dirs
	@echo " * [ALSA] ... linking $(LNKFLAGS) ($(SND_ALSA_LIBS))"; \
		$(CC) $(LDFLAGS) $(SHARED_LDFLAGS) -o $@ $(SND_ALSA_OBJS) $(SND_ALSA_LIBS) $(LNKFLAGS)

# Say how to build .o files from .c files for this module
$(BUILDDIR)/snd-alsa/%.o: $(SRCDIR)/%.c $(BUILDDIR)/.dirs
	@echo " * [ALSA] $<"; \
		$(CC) $(CFLAGS) $(CPPFLAGS) $(SHARED_CFLAGS) -o $@ -c $< -MD -MT $@ -MP

###################################################################################################
# JACK
###################################################################################################

SND_JACK_SRCS=ports/linux/linux_snd_jack.c
SND_JACK_OBJS=$(SND_JACK_SRCS:%.c=$(BUILDDIR)/snd-jack/%.o)
SND_JACK_TARGET=snd_jack.$(SHARED_EXT)

ifeq ($(BUILD_CLIENT), 1)
ifeq ($(HAVE_SND_JACK),1)
	TARGETS += $(SND_JACK_TARGET)
	ALL_OBJS += $(SND_JACK_OBJS)
endif
endif

# Say about to build the target
$(SND_JACK_TARGET) : $(SND_JACK_OBJS) $(BUILDDIR)/.dirs
	@echo " * [JACK] ... linking $(LNKFLAGS) ($(SND_JACK_LIBS))"; \
		$(CC) $(LDFLAGS) $(SHARED_LDFLAGS) -o $@ $(SND_JACK_OBJS) $(SND_JACK_LIBS) $(LNKFLAGS)

# Say how to build .o files from .c files for this module
$(BUILDDIR)/snd-jack/%.o: $(SRCDIR)/%.c $(BUILDDIR)/.dirs
	@echo " * [JACK] $<"; \
		$(CC) $(CFLAGS) $(CPPFLAGS) $(SHARED_CFLAGS) -o $@ -c $< -MD -MT $@ -MP

###################################################################################################
# OSS
###################################################################################################

SND_OSS_SRCS=ports/linux/linux_snd_oss.c
SND_OSS_OBJS=$(SND_OSS_SRCS:%.c=$(BUILDDIR)/snd-oss/%.o)
SND_OSS_TARGET=snd_oss.$(SHARED_EXT)

ifeq ($(BUILD_CLIENT), 1)
ifeq ($(HAVE_SND_OSS),1)
	TARGETS += $(SND_OSS_TARGET)
	ALL_OBJS += $(SND_OSS_OBJS)
endif
endif

# Say about to build the target
$(SND_OSS_TARGET) : $(SND_OSS_OBJS) $(BUILDDIR)/.dirs
	@echo " * [OSS] ... linking $(LNKFLAGS) ($(SND_OSS_LIBS))"; \
		$(CC) $(LDFLAGS) $(SHARED_LDFLAGS) -o $@ $(SND_OSS_OBJS) $(SND_OSS_LIBS) $(LNKFLAGS)

# Say how to build .o files from .c files for this module
$(BUILDDIR)/snd-oss/%.o: $(SRCDIR)/%.c $(BUILDDIR)/.dirs
	@echo " * [OSS] $<"; \
		$(CC) $(CFLAGS) $(CPPFLAGS) $(SHARED_CFLAGS) -o $@ -c $< -MD -MT $@ -MP

###################################################################################################
# ARTS
###################################################################################################

SND_ARTS_SRCS=ports/linux/linux_snd_arts.c
SND_ARTS_OBJS=$(SND_ARTS_SRCS:%.c=$(BUILDDIR)/snd-arts/%.o)
SND_ARTS_TARGET=snd_arts.$(SHARED_EXT)

ifeq ($(BUILD_CLIENT), 1)
ifeq ($(HAVE_SND_ARTS),1)
	TARGETS += $(SND_ARTS_TARGET)
	ALL_OBJS += $(SND_ARTS_OBJS)
endif
endif

# Say about to build the target
$(SND_ARTS_TARGET) : $(SND_ARTS_OBJS) $(BUILDDIR)/.dirs
	@echo " * [ARTS] ... linking $(LNKFLAGS) ($(SND_ARTS_LIBS))"; \
		$(CC) $(LDFLAGS) $(SHARED_LDFLAGS) -o $@ $(SND_ARTS_OBJS) $(LNKFLAGS) $(SND_ARTS_LIBS)

# Say how to build .o files from .c files for this module
$(BUILDDIR)/snd-arts/%.o: $(SRCDIR)/%.c $(BUILDDIR)/.dirs
	@echo " * [ARTS] $<"; \
		$(CC) $(CFLAGS) $(CPPFLAGS) $(SHARED_CFLAGS) $(SND_ARTS_CFLAGS) -o $@ -c $< -MD -MT $@ -MP

###################################################################################################
# WAPI
###################################################################################################

ifeq ($(TARGET_OS),mingw32)
	SND_WAPI_SRCS=ports/windows/win_snd_wapi.c
	SND_WAPI_OBJS=$(SND_WAPI_SRCS:%.c=$(BUILDDIR)/snd-wapi/%.o)
	SND_WAPI_TARGET=snd_wapi.$(SHARED_EXT)

	ifeq ($(BUILD_CLIENT), 1)
	ifeq ($(HAVE_SND_WAPI),1)
		TARGETS += $(SND_WAPI_TARGET)
		ALL_OBJS += $(SND_WAPI_OBJS)
	endif
	endif

	# Say about to build the target
	$(SND_WAPI_TARGET) : $(SND_WAPI_OBJS) $(BUILDDIR)/.dirs
		@echo " * [WAPI] ... linking $(LNKFLAGS) ($(SND_WAPI_LIBS))"; \
			$(CC) $(LDFLAGS) $(SHARED_LDFLAGS) -o $@ $(SND_WAPI_OBJS) $(SND_WAPI_LIBS) $(LNKFLAGS)

	# Say how to build .o files from .c files for this module
	$(BUILDDIR)/snd-wapi/%.o: $(SRCDIR)/%.c $(BUILDDIR)/.dirs
		@echo " * [WAPI] $<"; \
			$(CC) $(CFLAGS) $(CPPFLAGS) $(SHARED_CFLAGS) -o $@ -c $< -MD -MT $@ -MP

endif

###################################################################################################
# OSX sound driver
###################################################################################################
ifeq ($(TARGET_OS),darwin)
	SND_OSX_SRCS=ports/macosx/osx_snd_native.c
	SND_OSX_OBJS=$(SND_OSX_SRCS:%.c=$(BUILDDIR)/snd-osx/%.o)
	SND_OSX_TARGET=snd_osx.$(SHARED_EXT)

	ifeq ($(BUILD_CLIENT), 1)
	ifeq ($(HAVE_SND_OSX),1)
		TARGETS += $(SND_OSX_TARGET)
		ALL_OBJS += $(SND_OSX_OBJS)
	endif
	endif

	# Say about to build the target
	$(SND_OSX_TARGET) : $(SND_OSX_OBJS) $(BUILDDIR)/.dirs
		@echo " * [OSX] ... linking $(LNKFLAGS) ($(SND_OSX_LIBS))"; \
			$(CC) $(LDFLAGS) $(SHARED_LDFLAGS) -o $@ $(SND_OSX_OBJS) $(SND_OSX_LIBS) $(LNKFLAGS)

	# Say how to build .o files from .c files for this module
	$(BUILDDIR)/snd-osx/%.o: $(SRCDIR)/%.c $(BUILDDIR)/.dirs
		@echo " * [OSX] $<"; \
			$(CC) $(CFLAGS) $(CPPFLAGS) $(SHARED_CFLAGS) -o $@ -c $< -MD -MT $@ -MP

endif
