EFFECTS =	Quasi
#CC = gcc
LD = gcc

DEFINES =	-DPremultipliedARGB32Pixels

C_OPTS =	-std=c99 \
		-Wall \
		-Werror \
		-Wno-unused-function \
		-O3 \
		-ffast-math \
		-mfpmath=sse \
		-march=atom \
		-mtune=atom \
		-msse -msse2 -msse3 -msse4 -msse4.1 -msse4.2 -mmmx

LIBS =	-lm

SOURCE_DIR = .
BUILD_DIR = Build

COMMON_C_FILES= Engine/Downsample.c \
		Engine/GIF.c \
		Engine/LZW.c \
		Engine/Main.c \
		Engine/Neuquant.c \
		Engine/PNG.c \
		Engine/Quantize.c \
		Engine/Random.c \
		Graphics/Bitmap.c \
		Graphics/ColourFont.c \
		Graphics/DrawingBitmaps.c \
		Graphics/DrawingLines.c \
		Graphics/DrawingCircles.c \
		Graphics/DrawingPixels.c \
		Graphics/DrawingRectangles.c \
		Graphics/DrawingRLEBitmaps.c \
		Graphics/DrawingStraightLines.c \
		Graphics/Font.c \
		Graphics/MonoFont.c \
		Graphics/RLEBitmap.c \
		Vector/Matrix.c \
		Vector/MatrixDouble.c \
		Vector/Quaternion.c \
		Vector/QuaternionDouble.c \
		Vector/RandomVector.c \
		Vector/Vector.c \
		Vector/VectorDouble.c \

EFFECT_C_FILES = $(EFFECTS:%=%.c)

COMMON_OBJS = $(COMMON_C_FILES:%.c=$(BUILD_DIR)/%.o)

EFFECT_OBJS = $(EFFECT_C_FILES:%.c=$(BUILD_DIR)/%.o)

OBJS = $(COMMON_OBJS) $(EFFECT_OBJS)

ALL_CFLAGS = $(C_OPTS) $(DEFINES) $(CFLAGS)
ALL_LDFLAGS = $(LD_FLAGS)

AUTODEPENDENCY_CFLAGS=-MMD -MF$(@:.o=.d) -MT$@





all: $(EFFECTS)

clean:
	rm -rf $(BUILD_DIR) $(EFFECTS)

$(EFFECTS): %: $(BUILD_DIR)/%.o $(COMMON_OBJS)
	$(LD) $(ALL_LDFLAGS) -o $@ $^ $(LIBS)

.SUFFIXES: .o .c

$(BUILD_DIR)/%.o: $(SOURCE_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(ALL_CFLAGS) $(AUTODEPENDENCY_CFLAGS) -c $< -o $@

-include $(OBJS:.o=.d)
