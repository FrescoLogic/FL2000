// fl2000_interrupt.h
//
// (c)Copyright 2017, Fresco Logic, Incorporated.
//
// Purpose: Companion file.
//

#ifndef _FL2000_INTERRUPT_H_
#define _FL2000_INTERRUPT_H_

struct vga_status {
	union {
	struct  {
	uint32_t connected:1;
	uint32_t frame_dropped:1;
	uint32_t line_buffer_halted:1;

	// Change status due to ISO ACK received and a feedback return packet has been sent to ISO Interrupt IN EP.
	//
	uint32_t isoch_ack_changed:1;

        // Change status due to feedback algorithm has detected a drop of TD.
        //
        uint32_t feedback_dropped:1;
        uint32_t intr_pending:1;
        uint32_t pll_powered_up:1;
        uint32_t dac_powered_up:1;
        uint32_t line_buffer_underflow:1;
        uint32_t line_buffer_overflow:1;
        uint32_t frame_count:16;
        uint32_t hdmi_connection_changed:1;
        uint32_t hdmi_enabled:1;
        uint32_t edid_connected:1;
        uint32_t ext_mon_connected:1;
        uint32_t ext_mon_connect_changed:1;
        uint32_t edid_connect_changed:1;
	};

	uint32_t value;
	};
};


int fl2000_intr_pipe_create(struct dev_ctx * dev_ctx);
void fl2000_intr_pipe_destroy(struct dev_ctx * dev_ctx);

int fl2000_intr_pipe_start(struct dev_ctx * dev_ctx);    
void fl2000_intr_pipe_stop(struct dev_ctx * dev_ctx);

void fl2000_intr_pipe_completion(struct urb * urb);
void fl2000_intr_pipe_work(struct work_struct * work_item);
void fl2000_intr_process(struct dev_ctx * dev_ctx);

#endif // _FL2000_INTERRUPT_H_

// eof: fl2000_interrupt.h
//
