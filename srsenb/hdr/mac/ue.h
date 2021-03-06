/**
 *
 * \section COPYRIGHT
 *
 * Copyright 2013-2017 Software Radio Systems Limited
 *
 * \section LICENSE
 *
 * This file is part of srsLTE.
 *
 * srsUE is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * srsUE is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * A copy of the GNU Affero General Public License can be found in
 * the LICENSE file in the top-level directory of this distribution
 * and at http://www.gnu.org/licenses/.
 *
 */

#ifndef UE_H
#define UE_H

#include "srslte/common/log.h"
#include "srslte/common/pdu.h"
#include "srslte/common/mac_pcap.h"
#include "srslte/common/pdu_queue.h"
#include "srslte/interfaces/enb_interfaces.h"
#include "srslte/interfaces/sched_interface.h"
#include <pthread.h>
#include "mac/mac_metrics.h"

namespace srsenb {
  
class ue : public srslte::read_pdu_interface,
           public srslte::pdu_queue::process_callback
{
public:
  
  ue() : mac_msg_dl(20), mac_msg_ul(20), pdus(128) {
    rlc   = NULL; 
    log_h = NULL; 
    rnti  = 0; 
    pcap  = NULL;
    nof_failures   = 0;
    phr_counter    = 0;
    dl_cqi_counter = 0;
    is_phy_added = false; 
    for (int i=0;i<NOF_HARQ_PROCESSES;i++) {
      pending_buffers[i] = NULL; 
    }
    pthread_mutex_init(&mutex, NULL);
  }
  
  virtual ~ue() {
    for (int i=0;i<NOF_HARQ_PROCESSES;i++) {
      srslte_softbuffer_rx_free(&softbuffer_rx[i]);
      srslte_softbuffer_tx_free(&softbuffer_tx[i]);
    }
    pthread_mutex_destroy(&mutex);
  }
  
  void     reset();
  
  void     start_pcap(srslte::mac_pcap* pcap_);
  void     set_tti(uint32_t tti); 
  
  void     config(uint16_t rnti, uint32_t nof_prb, sched_interface *sched, rrc_interface_mac *rrc_, rlc_interface_mac *rlc, srslte::log *log_h);
  uint8_t* generate_pdu(sched_interface::dl_sched_pdu_t pdu[sched_interface::MAX_RLC_PDU_LIST], 
                    uint32_t nof_pdu_elems, uint32_t grant_size);
  
  srslte_softbuffer_tx_t* get_tx_softbuffer(uint32_t harq_process);
  srslte_softbuffer_rx_t* get_rx_softbuffer(uint32_t tti);
  
  bool     process_pdus(); 
  uint8_t *request_buffer(uint32_t tti, uint32_t len); 
  void     process_pdu(uint8_t *pdu, uint32_t nof_bytes, uint32_t tstamp);
  void     push_pdu(uint32_t tti, uint32_t len); 
  void     deallocate_pdu(uint32_t tti);
  
  uint32_t rl_failure();
  void     rl_failure_reset();

  void set_lcg(uint32_t lcid, uint32_t lcg);

  void metrics_read(srsenb::mac_metrics_t* metrics);
  void metrics_rx(bool crc, uint32_t tbs);
  void metrics_tx(bool crc, uint32_t tbs);
  void metrics_phr(float phr);
  void metrics_dl_cqi(uint32_t dl_cqi);


  bool is_phy_added;

private: 
  int  read_pdu(uint32_t lcid, uint8_t *payload, uint32_t requested_bytes);   
  void allocate_sdu(srslte::sch_pdu *pdu, uint32_t lcid, uint32_t sdu_len);   
  bool process_ce(srslte::sch_subh *subh); 
  void allocate_ce(srslte::sch_pdu *pdu, uint32_t lcid);

  std::vector<uint32_t> lc_groups[4];

  uint32_t phr_counter;
  uint32_t dl_cqi_counter;
  mac_metrics_t metrics;
  
  srslte::mac_pcap* pcap;
  
  uint64_t conres_id;
  
  uint16_t rnti; 
  
  uint32_t last_tti; 
  
  uint32_t nof_failures; 
  
  const static int NOF_HARQ_PROCESSES = 8; 
  srslte_softbuffer_tx_t softbuffer_tx[NOF_HARQ_PROCESSES];
  srslte_softbuffer_rx_t softbuffer_rx[NOF_HARQ_PROCESSES];

  uint8_t *pending_buffers[NOF_HARQ_PROCESSES]; 
  
  // For DL there is a single buffer 
  const static int payload_buffer_len = 128*1024; 
  uint8_t          tx_payload_buffer[payload_buffer_len];
  
  // For UL there are multiple buffers per PID and are managed by pdu_queue
  srslte::pdu_queue pdus; 
  srslte::sch_pdu mac_msg_dl, mac_msg_ul;
  
  rlc_interface_mac *rlc; 
  rrc_interface_mac* rrc;
  srslte::log *log_h;
  sched_interface* sched;
  
  bool conres_id_available;
  
  // Mutexes
  pthread_mutex_t mutex;
  
};

}

#endif

