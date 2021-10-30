/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2017 University of Padova
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Davide Magrin <magrinda@dei.unipd.it>
 *         Martina Capuzzo <capuzzom@dei.unipd.it>
 *
 * Modified by: Peggy Anderson <peggy.anderson@usask.ca>
 *              qiuyukang <b612n@qq.com>
 */

#include "ns3/class-a-end-device-lorawan-mac.h"
#include "ns3/end-device-lorawan-mac.h"
#include "ns3/end-device-lora-phy.h"
#include "ns3/log.h"
#include <algorithm>

namespace ns3
{
  namespace lorawan
  {

    NS_LOG_COMPONENT_DEFINE("ClassAEndDeviceLorawanMac");

    NS_OBJECT_ENSURE_REGISTERED(ClassAEndDeviceLorawanMac);

    TypeId
    ClassAEndDeviceLorawanMac::GetTypeId(void)
    {
      static TypeId tid = TypeId("ns3::ClassAEndDeviceLorawanMac")
                              .SetParent<EndDeviceLorawanMac>()
                              .SetGroupName("lorawan")
                              .AddConstructor<ClassAEndDeviceLorawanMac>()
                              .AddTraceSource("NumberOfFramesSent",
                                              "The number of Frames sent from this node.",
                                              MakeTraceSourceAccessor(&ClassAEndDeviceLorawanMac::TransmissionsSent),
                                              "ns3::TracedValueCallback::Int32")
                              .AddAttribute("FailedTransmissionCount",
                                            "The number of transmissions that didn't get an ACK.",
                                            DoubleValue(0),
                                            MakeIntegerAccessor(&ClassAEndDeviceLorawanMac::UnacknowledgedTransmissions),
                                            MakeIntegerChecker<uint32_t>())
                              .AddAttribute("LastFitnessLevel",
                                            "The fitness level of the last tx paramset used to tx.",
                                            DoubleValue(0),
                                            MakeDoubleAccessor(&ClassAEndDeviceLorawanMac::m_lastFitnessLevel),
                                            MakeDoubleChecker<float>())
                              .AddAttribute("UseGeneticAlgorithm",
                                            "Whether or not to use the Genetic Algorithm for tx param optimization.",
                                            BooleanValue(false),
                                            MakeBooleanAccessor(&ClassAEndDeviceLorawanMac::useGeneticParamaterSelection),
                                            MakeBooleanChecker())
                              .AddAttribute("PacketErrorRate",
                                            "The factor of packets that are NOT acknowledged vs number of packets sent out.",
                                            DoubleValue(false),
                                            MakeDoubleAccessor(&ClassAEndDeviceLorawanMac::PacketErrorRate),
                                            MakeDoubleChecker<float>())
                              .AddAttribute("LastNPSR",
                                            "The PSR of the last N (32) frames sent. Interval set externally.",
                                            DoubleValue(false),
                                            MakeDoubleAccessor(&ClassAEndDeviceLorawanMac::lastNPacketSuccessRate),
                                            MakeDoubleChecker<float>())
                              .AddAttribute("TotalPowerConsumption",
                                            "Total accumulated power consumption of this node.",
                                            DoubleValue(false),
                                            MakeDoubleAccessor(&ClassAEndDeviceLorawanMac::TotalPowerConsumption),
                                            MakeDoubleChecker<float>())
                              .AddAttribute("TransmissionsSent",
                                            "Total number of transmissions sent by this node. Includes retransmissions.",
                                            IntegerValue(false),
                                            MakeIntegerAccessor(&ClassAEndDeviceLorawanMac::TransmissionsSent),
                                            MakeIntegerChecker<uint32_t>())              
                              ;

      return tid;
    }

    ClassAEndDeviceLorawanMac::ClassAEndDeviceLorawanMac() : // LoraWAN default
                                                             m_receiveDelay1(Seconds(1)),
                                                             // LoraWAN default
                                                             m_receiveDelay2(Seconds(2)),
                                                             m_rx1DrOffset(0)
    {
      NS_LOG_FUNCTION(this);

      // Void the two receiveWindow events
      m_closeFirstWindow = EventId();
      m_closeFirstWindow.Cancel();
      m_closeSecondWindow = EventId();
      m_closeSecondWindow.Cancel();
      m_secondReceiveWindow = EventId();
      m_secondReceiveWindow.Cancel();

      geneticTXParameterOptimizer = new GeneticTXParameterOptimizer();
    }

    ClassAEndDeviceLorawanMac::~ClassAEndDeviceLorawanMac()
    {
      NS_LOG_FUNCTION_NOARGS();
    }

    /////////////////////
    // Sending methods //
    /////////////////////

    void
    ClassAEndDeviceLorawanMac::SendToPhy(Ptr<Packet> packetToSend)
    {
      TransmissionsSent++;
      PacketErrorRate = static_cast<float>(UnacknowledgedTransmissions) / static_cast<float>(TransmissionsSent);

      /////////////////////////////////////////////////////////
      // Add headers, prepare TX parameters and send the packet
      /////////////////////////////////////////////////////////

      //NS_LOG_DEBUG ("PacketToSend: " << packetToSend);

      //NS_LOG_LOGIC("Sending a packet to PHY.");

      //here is where we need to select and individual to test.
      //we aught to have a list of 8 or so individuals and an int representing the index of the current one.
      //

      // Data Rate Adaptation as in LoRaWAN specification, V1.0.2 (2016)
      if (m_enableDRAdapt && (m_dataRate > 0) && (m_retxParams.retxLeft < m_maxNumbTx) && (m_retxParams.retxLeft % 2 == 0))
      {
        m_txPower = 14; // Reset transmission power
        m_dataRate = m_dataRate - 1;
        NS_LOG_INFO("m_enableDRAdapt");
      }

      // Craft LoraTxParameters object
      LoraTxParameters params;

      if (useGeneticParamaterSelection)
      {
        if(geneticTXParameterOptimizer->IsOptimizing()) {
          SetMType(LorawanMacHeader::CONFIRMED_DATA_UP);
        }else{
          SetMType(LorawanMacHeader::UNCONFIRMED_DATA_UP);
        }
        geneticTXParameterOptimizer->GetCurrentTransmissionParameterSet()->Print();
        m_lastFitnessLevel = geneticTXParameterOptimizer->GetCurrentTransmissionParameterSet()->fitness();
        //TotalPowerConsumption += geneticTXParameterOptimizer->GetCurrentTransmissionParameterSet()->PowerConsumption();
        m_txPower = geneticTXParameterOptimizer->GetCurrentTransmissionParameterSet()->power;
        params.sf = geneticTXParameterOptimizer->GetCurrentTransmissionParameterSet()->spreadingFactor;
        params.codingRate = geneticTXParameterOptimizer->GetCurrentTransmissionParameterSet()->codingRate;
        params.bandwidthHz = geneticTXParameterOptimizer->GetCurrentTransmissionParameterSet()->bandwidth;
        //manually set the datarate:
        //This is PROBABLY unecessary. ADRACKREQ's require datarates to be set like this, but the device
        //  should still be able to listen without messing around with DR stuff.  
        //Let's remove this chunk and see if things still work w/o adr.
        if (params.sf == 12)
        {
          m_dataRate = 0;
        }
        if (params.sf == 11)
        {
          m_dataRate = 1;
        }
        if (params.sf == 10)
        {
          m_dataRate = 2;
        }
        if (params.sf == 9)
        {
          m_dataRate = 3;
        }
        if (params.sf == 8)
        {
          m_dataRate = 4;
        }
        if (params.sf == 7 && params.bandwidthHz == 125000)
        {
          m_dataRate = 5;
        }
        if (params.sf == 7 && params.bandwidthHz == 250000)
        {
          m_dataRate = 6;
        }
      }
      else
      {
        params.sf = GetSfFromDataRate(m_dataRate);
        params.codingRate = m_codingRate;
        params.bandwidthHz = GetBandwidthFromDataRate(m_dataRate);
        //TotalPowerConsumption += TransmissionParameterSet::PowerConsumption(params.sf, params.bandwidthHz, params.codingRate, m_txPower);
        //std::cout << "SF: " << +params.sf << " TP: " << +m_txPower << " BW: " << +params.bandwidthHz << " CR: " << +params.codingRate << std::endl;
        //m_lastFitnessLevel = TransmissionParameterSet::fitness(params.sf, params.bandwidthHz, params.codingRate, m_txPower);
      }

      params.headerDisabled = m_headerDisabled;
      params.nPreamble = m_nPreambleSymbols;
      params.crcEnabled = 1;
      params.lowDataRateOptimizationEnabled = LoraPhy::GetTSym(params) > MilliSeconds(16) ? true : false;

      lastParams.sf = params.sf;
      lastParams.codingRate = params.codingRate;
      lastParams.bandwidthHz = params.bandwidthHz;

      // Wake up PHY layer and directly send the packet
      Ptr<LogicalLoraChannel> txChannel = GetChannelForTx();

      //NS_LOG_DEBUG ("PacketToSend: " << packetToSend);
      m_phy->Send(packetToSend, params, txChannel->GetFrequency(), m_txPower);

      //////////////////////////////////////////////
      // Register packet transmission for duty cycle
      //////////////////////////////////////////////

      // Compute packet duration
      Time duration = m_phy->GetOnAirTime(packetToSend, params);

      //Keep track of how much power we're consuming.
      TotalPowerConsumption += duration.GetSeconds() * m_txPower;

      // Register the sent packet into the DutyCycleHelper
      m_channelHelper.AddEvent(duration, txChannel);

      //////////////////////////////
      // Prepare for the downlink //
      //////////////////////////////

      // Switch the PHY to the channel so that it will listen here for downlink
      m_phy->GetObject<EndDeviceLoraPhy>()->SetFrequency(txChannel->GetFrequency());

      // Instruct the PHY on the right Spreading Factor to listen for during the window
      // create a SetReplyDataRate function?
      uint8_t replyDataRate = GetFirstReceiveWindowDataRate();
      NS_LOG_INFO("m_dataRate: " << unsigned(m_dataRate) << ", m_rx1DrOffset: " << unsigned(m_rx1DrOffset) << ", replyDataRate: " << unsigned(replyDataRate) << ".");


      //TODO: Figure out if this is even valid? GetSFFromDataRate might be necessary for 
      //  non-ADR tx's as well as ADR'd ones.
      //if (useGeneticParamaterSelection)
      //{
      //  m_phy->GetObject<EndDeviceLoraPhy>()->SetSpreadingFactor(params.sf);
      //}
      //else
      //{
        m_phy->GetObject<EndDeviceLoraPhy>()->SetSpreadingFactor(GetSfFromDataRate(replyDataRate));
      //}
    }

    //////////////////////////
    //  Receiving methods   //
    //////////////////////////
    void
    ClassAEndDeviceLorawanMac::Receive(Ptr<Packet const> packet)
    {
      //NS_LOG_FUNCTION (this << packet);

      //NS_LOG_LOGIC("Recieving a packet!");

      // Work on a copy of the packet
      Ptr<Packet> packetCopy = packet->Copy();

      // Remove the Mac Header to get some information
      LorawanMacHeader mHdr;
      packetCopy->RemoveHeader(mHdr);

      //NS_LOG_DEBUG ("Mac Header: " << mHdr);

      // Only keep analyzing the packet if it's downlink
      if (!mHdr.IsUplink())
      {
        //NS_LOG_INFO ("Found a downlink packet.");

        // Remove the Frame Header
        LoraFrameHeader fHdr;
        fHdr.SetAsDownlink();
        packetCopy->RemoveHeader(fHdr);

        //NS_LOG_DEBUG ("Frame Header: " << fHdr);

        // Determine whether this packet is for us
        bool messageForUs = (m_address == fHdr.GetAddress());

        if (messageForUs)
        {
          //NS_LOG_INFO ("The message is for us!");

          // If it exists, cancel the second receive window event
          // THIS WILL BE GetReceiveWindow()
          Simulator::Cancel(m_secondReceiveWindow);

          // Parse the MAC commands
          ParseCommands(fHdr);

          // TODO Pass the packet up to the NetDevice

          // Call the trace source
          m_receivedPacket(packet);
        }
        else
        {
          //NS_LOG_DEBUG ("The message is intended for another recipient.");

          // In this case, we are either receiving in the first receive window
          // and finishing reception inside the second one, or receiving a
          // packet in the second receive window and finding out, after the
          // fact, that the packet is not for us. In either case, if we no
          // longer have any retransmissions left, we declare failure.
          if (m_retxParams.waitingAck && m_secondReceiveWindow.IsExpired())
          {
            if (m_retxParams.retxLeft == 0)
            {
              uint8_t txs = m_maxNumbTx - (m_retxParams.retxLeft);
              m_requiredTxCallback(txs, false, m_retxParams.firstAttempt, m_retxParams.packet);
              NS_LOG_DEBUG("Failure: no more retransmissions left. Used " << unsigned(txs) << " transmissions.");
              AckNotRecieved();

              // Reset retransmission parameters
              resetRetransmissionParameters();
            }
            else // Reschedule
            {
              this->Send(m_retxParams.packet);
              NS_LOG_INFO("We have " << unsigned(m_retxParams.retxLeft) << " retransmissions left: rescheduling transmission.");
              AckNotRecieved();
            }
          }
        }
      }
      else if (m_retxParams.waitingAck && m_secondReceiveWindow.IsExpired())
      {
        //NS_LOG_INFO ("The packet we are receiving is in uplink.");
        if (m_retxParams.retxLeft > 0)
        {
          this->Send(m_retxParams.packet);
          //NS_LOG_INFO ("We have " << unsigned(m_retxParams.retxLeft) << " retransmissions left: rescheduling transmission.");
          AckNotRecieved();
        }
        else
        {
          uint8_t txs = m_maxNumbTx - (m_retxParams.retxLeft);
          m_requiredTxCallback(txs, false, m_retxParams.firstAttempt, m_retxParams.packet);
          //NS_LOG_DEBUG ("Failure: no more retransmissions left. Used " << unsigned(txs) << " transmissions.");
          AckNotRecieved();

          //NOTE: This is where the packet totally fails after 8 attempts. ??? (IS it? There are 3 other spots where this might be happening too)

          // Reset retransmission parameters
          resetRetransmissionParameters();
        }
      }

      m_phy->GetObject<EndDeviceLoraPhy>()->SwitchToSleep();
    }

    void
    ClassAEndDeviceLorawanMac::FailedReception(Ptr<Packet const> packet)
    {
      //NS_LOG_FUNCTION (this << packet);

      //NS_LOG_LOGIC("Failed Reception.");

      // Switch to sleep after a failed reception
      m_phy->GetObject<EndDeviceLoraPhy>()->SwitchToSleep();

      if (m_secondReceiveWindow.IsExpired() && m_retxParams.waitingAck)
      {
        if (m_retxParams.retxLeft > 0)
        {
          this->Send(m_retxParams.packet);
          //NS_LOG_INFO ("We have " << unsigned(m_retxParams.retxLeft) << " retransmissions left: rescheduling transmission.");
          AckNotRecieved();
        }
        else
        {
          uint8_t txs = m_maxNumbTx - (m_retxParams.retxLeft);
          m_requiredTxCallback(txs, false, m_retxParams.firstAttempt, m_retxParams.packet);
          //NS_LOG_DEBUG ("Failure: no more retransmissions left. Used " << unsigned(txs) << " transmissions.");
          AckNotRecieved();

          // Reset retransmission parameters
          resetRetransmissionParameters();
        }
      }
    }

    void
    ClassAEndDeviceLorawanMac::TxFinished(Ptr<const Packet> packet)
    {
      //NS_LOG_FUNCTION_NOARGS ();

      // Schedule the opening of the first receive window
      Simulator::Schedule(m_receiveDelay1,
                          &ClassAEndDeviceLorawanMac::OpenFirstReceiveWindow, this);

      // Schedule the opening of the second receive window
      m_secondReceiveWindow = Simulator::Schedule(m_receiveDelay2,
                                                  &ClassAEndDeviceLorawanMac::OpenSecondReceiveWindow,
                                                  this);
      // // Schedule the opening of the first receive window
      // Simulator::Schedule (m_receiveDelay1,
      //                      &ClassAEndDeviceLorawanMac::OpenFirstReceiveWindow, this);
      //
      // // Schedule the opening of the second receive window
      // m_secondReceiveWindow = Simulator::Schedule (m_receiveDelay2,
      //                                              &ClassAEndDeviceLorawanMac::OpenSecondReceiveWindow,
      //                                              this);

      // Switch the PHY to sleep
      m_phy->GetObject<EndDeviceLoraPhy>()->SwitchToSleep();
    }

    void
    ClassAEndDeviceLorawanMac::OpenFirstReceiveWindow(void)
    {
      //NS_LOG_FUNCTION_NOARGS ();

      // Set Phy in Standby mode
      m_phy->GetObject<EndDeviceLoraPhy>()->SwitchToStandby();

      //Calculate the duration of a single symbol for the first receive window DR
      double tSym = pow(2, GetSfFromDataRate(GetFirstReceiveWindowDataRate())) / GetBandwidthFromDataRate(GetFirstReceiveWindowDataRate());

      // Schedule return to sleep after "at least the time required by the end
      // device's radio transceiver to effectively detect a downlink preamble"
      // (LoraWAN specification)
      m_closeFirstWindow = Simulator::Schedule(Seconds(m_receiveWindowDurationInSymbols * tSym),
                                               &ClassAEndDeviceLorawanMac::CloseFirstReceiveWindow, this); //m_receiveWindowDuration
    }

    void
    ClassAEndDeviceLorawanMac::CloseFirstReceiveWindow(void)
    {
      //NS_LOG_FUNCTION_NOARGS ();

      Ptr<EndDeviceLoraPhy> phy = m_phy->GetObject<EndDeviceLoraPhy>();

      // Check the Phy layer's state:
      // - RX -> We are receiving a preamble.
      // - STANDBY -> Nothing was received.
      // - SLEEP -> We have received a packet.
      // We should never be in TX or SLEEP mode at this point
      switch (phy->GetState())
      {
      case EndDeviceLoraPhy::TX:
        NS_ABORT_MSG("PHY was in TX mode when attempting to "
                     << "close a receive window.");
        break;
      case EndDeviceLoraPhy::RX:
        // PHY is receiving: let it finish. The Receive method will switch it back to SLEEP.
        break;
      case EndDeviceLoraPhy::SLEEP:
        // PHY has received, and the MAC's Receive already put the device to sleep
        break;
      case EndDeviceLoraPhy::STANDBY:
        // Turn PHY layer to SLEEP
        phy->SwitchToSleep();
        break;
      }
    }

    void
    ClassAEndDeviceLorawanMac::OpenSecondReceiveWindow(void)
    {
      //NS_LOG_FUNCTION_NOARGS ();

      // Check for receiver status: if it's locked on a packet, don't open this
      // window at all.
      if (m_phy->GetObject<EndDeviceLoraPhy>()->GetState() == EndDeviceLoraPhy::RX)
      {
        NS_LOG_INFO("Won't open second receive window since we are in RX mode.");

        return;
      }

      // Set Phy in Standby mode
      m_phy->GetObject<EndDeviceLoraPhy>()->SwitchToStandby();

      // Switch to appropriate channel and data rate
      //NS_LOG_INFO ("Using parameters: " << m_secondReceiveWindowFrequency << "Hz, DR" << unsigned(m_secondReceiveWindowDataRate));

      m_phy->GetObject<EndDeviceLoraPhy>()->SetFrequency(m_secondReceiveWindowFrequency);
      m_phy->GetObject<EndDeviceLoraPhy>()->SetSpreadingFactor(GetSfFromDataRate(m_secondReceiveWindowDataRate));

      //Calculate the duration of a single symbol for the second receive window DR
      double tSym = pow(2, GetSfFromDataRate(GetSecondReceiveWindowDataRate())) / GetBandwidthFromDataRate(GetSecondReceiveWindowDataRate());

      // Schedule return to sleep after "at least the time required by the end
      // device's radio transceiver to effectively detect a downlink preamble"
      // (LoraWAN specification)
      m_closeSecondWindow = Simulator::Schedule(Seconds(m_receiveWindowDurationInSymbols * tSym),
                                                &ClassAEndDeviceLorawanMac::CloseSecondReceiveWindow, this);
    }

    void
    ClassAEndDeviceLorawanMac::CloseSecondReceiveWindow(void)
    {
      //NS_LOG_FUNCTION_NOARGS ();

      Ptr<EndDeviceLoraPhy> phy = m_phy->GetObject<EndDeviceLoraPhy>();

      // NS_ASSERT (phy->m_state != EndDeviceLoraPhy::TX &&
      // phy->m_state != EndDeviceLoraPhy::SLEEP);

      // Check the Phy layer's state:
      // - RX -> We have received a preamble.
      // - STANDBY -> Nothing was detected.
      switch (phy->GetState())
      {
      case EndDeviceLoraPhy::TX:
        break;
      case EndDeviceLoraPhy::SLEEP:
        break;
      case EndDeviceLoraPhy::RX:
        // PHY is receiving: let it finish
        //NS_LOG_DEBUG ("PHY is receiving: Receive will handle the result.");
        return;
      case EndDeviceLoraPhy::STANDBY:
        // Turn PHY layer to sleep
        phy->SwitchToSleep();
        break;
      }

      if (m_retxParams.waitingAck)
      {
        //NS_LOG_DEBUG ("No reception initiated by PHY: rescheduling transmission.");
        if (m_retxParams.retxLeft > 0)
        {
          //NS_LOG_INFO ("We have " << unsigned(m_retxParams.retxLeft) << " retransmissions left: rescheduling transmission.");
          AckNotRecieved();
          this->Send(m_retxParams.packet);
        }
        //we have no retransmissions left and we aren't detecting a preamble...
        else if (m_retxParams.retxLeft == 0 && m_phy->GetObject<EndDeviceLoraPhy>()->GetState() != EndDeviceLoraPhy::RX)
        {
          uint8_t txs = m_maxNumbTx - (m_retxParams.retxLeft);
          m_requiredTxCallback(txs, false, m_retxParams.firstAttempt, m_retxParams.packet);
          //NS_LOG_DEBUG ("Failure: no more retransmissions left. Used " << unsigned(txs) << " transmissions.");
          AckNotRecieved();
          //NOTE: This seems to be where the packet ACK timeout occurs. This should be where we do our ML operation.
          //BUT, the system does 8 retransmissions. Each of these should learn as well!

          // Reset retransmission parameters
          resetRetransmissionParameters();
        }

        else
        {
          NS_ABORT_MSG("The number of retransmissions left is negative ! ");
        }
      }
      else
      {
        uint8_t txs = m_maxNumbTx - (m_retxParams.retxLeft);
        m_requiredTxCallback(txs, true, m_retxParams.firstAttempt, m_retxParams.packet);
        //NS_LOG_INFO ("We have " << unsigned(m_retxParams.retxLeft) << " transmissions left. We were not transmitting confirmed messages.");

        // Reset retransmission parameters
        resetRetransmissionParameters();
      }
    }

    void
    ClassAEndDeviceLorawanMac::AckNotRecieved()
    {
      setLastFrameSuccess(false);

      UnacknowledgedTransmissions++;
      PacketErrorRate = static_cast<float>(UnacknowledgedTransmissions) / static_cast<float>(TransmissionsSent);

      if (useGeneticParamaterSelection)
      {
        geneticTXParameterOptimizer->SetCurrentTransmissionParameterSetSuccess(false);
      }
    }

    void
    ClassAEndDeviceLorawanMac::recievedAck()
    {
      setLastFrameSuccess(true);

      if (useGeneticParamaterSelection)
      {

        NS_LOG_INFO("Successfully Recieved Ack from Transmission with fitness: " << geneticTXParameterOptimizer->GetCurrentTransmissionParameterSet()->fitness());
        geneticTXParameterOptimizer->SetCurrentTransmissionParameterSetSuccess(true);
      }
    }

    void
    ClassAEndDeviceLorawanMac::setLastFrameSuccess(bool success)
    {
      for (int i = 0; i < 31; i++)
      {
        lastNTransmissionSuccess[i] = lastNTransmissionSuccess[i + 1];
      }
      lastNTransmissionSuccess[31] = success;
      if (TransmissionsSent.Get() >= 32)
      {
        double successful = 0;
        for (int i = 0; i < 32; i++)
        {
          if (lastNTransmissionSuccess[i] == true)
          {
            successful += 1.0;
          }
        }
        lastNPacketSuccessRate = successful / 32.0;
      }
    }

    /////////////////////////
    // Getters and Setters //
    /////////////////////////

    Time
    ClassAEndDeviceLorawanMac::GetNextClassTransmissionDelay(Time waitingTime)
    {
      //NS_LOG_FUNCTION_NOARGS ();

      // This is a new packet from APP; it can not be sent until the end of the
      // second receive window (if the second recieve window has not closed yet)
      if (!m_retxParams.waitingAck)
      {
        if (!m_closeFirstWindow.IsExpired() ||
            !m_closeSecondWindow.IsExpired() ||
            !m_secondReceiveWindow.IsExpired())
        {
          NS_LOG_WARN("Attempting to send when there are receive windows:"
                      << " Transmission postponed.");
          // Compute the duration of a single symbol for the second receive window DR
          double tSym = pow(2, GetSfFromDataRate(GetSecondReceiveWindowDataRate())) / GetBandwidthFromDataRate(GetSecondReceiveWindowDataRate());
          // Compute the closing time of the second receive window
          Time endSecondRxWindow = Time(m_secondReceiveWindow.GetTs()) + Seconds(m_receiveWindowDurationInSymbols * tSym);

          NS_LOG_DEBUG("Duration until endSecondRxWindow for new transmission:" << (endSecondRxWindow - Simulator::Now()).GetSeconds());
          waitingTime = std::max(waitingTime, endSecondRxWindow - Simulator::Now());
        }
      }
      // This is a retransmitted packet, it can not be sent until the end of
      // ACK_TIMEOUT (this timer starts when the second receive window was open)
      else
      {
        double ack_timeout = m_uniformRV->GetValue(1, 3);
        // Compute the duration until ACK_TIMEOUT (It may be a negative number, but it doesn't matter.)
        Time retransmitWaitingTime = Time(m_secondReceiveWindow.GetTs()) - Simulator::Now() + Seconds(ack_timeout);

        //NS_LOG_DEBUG("ack_timeout:" << ack_timeout << " retransmitWaitingTime:" << retransmitWaitingTime.GetSeconds());
        waitingTime = std::max(waitingTime, retransmitWaitingTime);
      }

      return waitingTime;
    }

    uint8_t
    ClassAEndDeviceLorawanMac::GetFirstReceiveWindowDataRate(void)
    {
      return m_replyDataRateMatrix.at(m_dataRate).at(m_rx1DrOffset);
    }

    void
    ClassAEndDeviceLorawanMac::SetSecondReceiveWindowDataRate(uint8_t dataRate)
    {
      m_secondReceiveWindowDataRate = dataRate;
    }

    uint8_t
    ClassAEndDeviceLorawanMac::GetSecondReceiveWindowDataRate(void)
    {
      return m_secondReceiveWindowDataRate;
    }

    void
    ClassAEndDeviceLorawanMac::SetSecondReceiveWindowFrequency(double frequencyMHz)
    {
      m_secondReceiveWindowFrequency = frequencyMHz;
    }

    double
    ClassAEndDeviceLorawanMac::GetSecondReceiveWindowFrequency(void)
    {
      return m_secondReceiveWindowFrequency;
    }

    /////////////////////////
    // MAC command methods //
    /////////////////////////

    void
    ClassAEndDeviceLorawanMac::OnRxClassParamSetupReq(Ptr<RxParamSetupReq> rxParamSetupReq)
    {
      //NS_LOG_FUNCTION (this << rxParamSetupReq);

      bool offsetOk = true;
      bool dataRateOk = true;

      uint8_t rx1DrOffset = rxParamSetupReq->GetRx1DrOffset();
      uint8_t rx2DataRate = rxParamSetupReq->GetRx2DataRate();
      double frequency = rxParamSetupReq->GetFrequency();

      //NS_LOG_FUNCTION (this << unsigned (rx1DrOffset) << unsigned (rx2DataRate) << frequency);

      // Check that the desired offset is valid
      if (!(0 <= rx1DrOffset && rx1DrOffset <= 5))
      {
        offsetOk = false;
      }

      // Check that the desired data rate is valid
      if (GetSfFromDataRate(rx2DataRate) == 0 || GetBandwidthFromDataRate(rx2DataRate) == 0)
      {
        dataRateOk = false;
      }

      // For now, don't check for validity of frequency
      m_secondReceiveWindowDataRate = rx2DataRate;
      m_rx1DrOffset = rx1DrOffset;
      m_secondReceiveWindowFrequency = frequency;

      // Craft a RxParamSetupAns as response
      //NS_LOG_INFO ("Adding RxParamSetupAns reply");
      m_macCommandList.push_back(CreateObject<RxParamSetupAns>(offsetOk,
                                                               dataRateOk, true));
    }

  } /* namespace lorawan */
} /* namespace ns3 */
