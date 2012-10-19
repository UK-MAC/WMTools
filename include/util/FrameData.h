#ifndef FRAMEDATA
#define FRAMEDATA

using namespace std;

/**
 * The FrameData class is designed to store specific information about the size and content of output frames.
 *
 * This is the one location where the size is defined, enabling it to be easily changed.
 * For frames with variable data sizes we store a 'forward' size containing the size of the fixed portion of the data size.
 *
 * The object also contains a static definition of the flags used to represent each frame of the output.
 */
class FrameData {

private:

	/* FrameSizes */
	int malloc_frame_size;
	int calloc_frame_size;
	int realloc_frame_size;
	int free_frame_size;

	/* Partial frame sizes */
	int data_forward;
	int elf_forward;
	int virtual_forward;
	int stacks_forward;
	int cores_forward;

public:
	/* Static definitions of frame flags */
	static const char MALLOCFLAG = 'M';
	static const char CALLOCFLAG = 'C';
	static const char REALLOCFLAG = 'R';
	static const char FREEFLAG = 'F';

	static const char STACKFLAG = 'S';
	static const char ELFFLAG = 'E';
	static const char VIRTUALFLAG = 'V';
	static const char DATAFLAG = 'D';
	static const char FINISHFLAG = 'Z';
	static const char CORESFLAG = 'C';

	/**
	 * Constructor for frame data.
	 * Set the value of the frame sizes.
	 */
	FrameData();

	/**
	 * Getter for Calloc frame size
	 * @return Calloc frame size
	 */
	int getCallocFrameSize() const {
		return calloc_frame_size;
	}

	/**
	 * Getter for Free frame size
	 * @return Free frame size
	 */
	int getFreeFrameSize() const {
		return free_frame_size;
	}

	/**
	 * Getter for Malloc frame size
	 * @return Malloc frame size
	 */
	int getMallocFrameSize() const {
		return malloc_frame_size;
	}

	/**
	 * Getter for Realloc frame size
	 * @return Realloc frame size
	 */
	int getReallocFrameSize() const {
		return realloc_frame_size;
	}

	/**
	 * Getter for the partial size of the Data frame.
	 * Before the remaining data is added.
	 * @return The initial size of the data frame.
	 */
	int getDataForward() const {
		return data_forward;
	}

	/**
	 * Getter for the partial size of the Elf frame.
	 * Before the remaining data is added.
	 * @return The initial size of the elf frame.
	 */
	int getElfForward() const {
		return elf_forward;
	}

	/**
	 * Getter for the partial size of the Virtual functions frame.
	 * Before the remaining data is added.
	 * @return The initial size of the virtual functions frame.
	 */
	int getVirtualForward() const {
		return virtual_forward;
	}

	/**
	 * Getter for the partial size of the Call Stacks frame.
	 *
	 * @return The initial size of the Call Stacks frame.
	 */
	int getStacksForward() const {
		return stacks_forward;
	}

	/**
	 * Getter for the partial size of the Call Stacks frame.
	 *
	 * @return The initial size of the Call Stacks frame.
	 */
	int getCoresForward() const {
		return cores_forward;
	}
};

#endif
