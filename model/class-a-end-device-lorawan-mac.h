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
 */

#ifndef CLASS_A_END_DEVICE_LORAWAN_MAC_H
#define CLASS_A_END_DEVICE_LORAWAN_MAC_H

#include "ns3/lorawan-mac.h"                // Packet
#include "ns3/end-device-lorawan-mac.h"     // EndDeviceLorawanMac
#include "ns3/lora-frame-header.h"          // RxParamSetupReq
// #include "ns3/random-variable-stream.h"
#include "ns3/lora-device-address.h"
// #include "ns3/traced-value.h"

namespace ns3 {
namespace lorawan {

class TXParameterIndividual {
  public:
    Ptr<UniformRandomVariable> randomGenerator;
    int power;
    int spreadingFactor;
    int bandwidth;
    int codingRate;
    bool successful = true; //we assume its succesful until we actually try to use it.
    
    TXParameterIndividual() {
      init();
      //spreading factor: 7 to 12
      spreadingFactor = randomGenerator->GetInteger(7, 12);
      //power: 2, 4, 6, 8, 10, 12, 14, 16
      power = randomGenerator->GetInteger(1, 7) * 2; //8
      //bandwidth: 125000, 250000, 500000
      int choice = randomGenerator->GetInteger(1, 3);
      if (choice == 1){
        bandwidth = 125000;
      }else if(choice == 2){
        bandwidth = 250000;
      }else {
        bandwidth = 500000;
      }
      //coding rate: 1 (4/5), 2 (4/6), 3 (4/7), 4 (4/8)
      codingRate = randomGenerator->GetInteger(1, 4);
      Print();
    }

    TXParameterIndividual(TXParameterIndividual* parent_a, TXParameterIndividual* parent_b){
      init();
      //generate parameters based on parent's parameters.
      int sf_choice = randomGenerator->GetInteger(1,2);
      if(sf_choice == 1){
        spreadingFactor = parent_a->spreadingFactor;
      }else{
        spreadingFactor = parent_b->spreadingFactor;
      }
      
      int pow_choice = randomGenerator->GetInteger(1,2);
      if(pow_choice == 1){
        power = parent_a->power;
      }else{
        power = parent_b->power;
      }

      int bw_choice = randomGenerator->GetInteger(1,2);
      if(bw_choice == 1){
        bandwidth = parent_a->bandwidth;
      }else{
        bandwidth = parent_b->bandwidth;
      }

      int cr_choice = randomGenerator->GetInteger(1,2);
      if(cr_choice == 1){
        codingRate = parent_a->codingRate;
      }else{
        codingRate = parent_b->codingRate;
      }

      int mutate_choice = randomGenerator->GetInteger(1, 100);
      if(mutate_choice > 95){
        //mutate, else do nothing.
      }
    }

    void init() {
      randomGenerator = CreateObject<UniformRandomVariable> ();
    }

    void Print() {
      std::cout << "TXPARAMS: SF=" << spreadingFactor << " PW=" << power << " BW=" << bandwidth << " CR=" << codingRate << " FITNESS=" << fitness() << std::endl;
    }

    float fitness() {
      //Output 0-1 depending on where the power factor is between the theoretical min and max.
      //lowest  = 0.0914286
      //highest = 87.3813
      //(v - lowest) / (highest - lowest)
      return 1.0f - (powerConsumption() - 0.0914286) / (87.3813 - 0.0914286);
      //return powerConsumption();
    }

    float powerConsumption() {
      //DataRate * Power
      //https://www.ncbi.nlm.nih.gov/pmc/articles/PMC7070984/
      //this function will calculate the amount of power needed to transmit 1000 bytes of data.
      //1000 is chosen so that values are large enough such that they do not need to be expressed in scientific notation.
      //the actual value choen is arbitrary and holds no bearing on results.
      return (1000.0f/dataRate()) * power;
    }

    float dataRate() {
      //return spreadingFactor * ((4.0f / 4.0f + codingRate) / ((2^spreadingFactor) / bandwidth ) ) * 1000.0f;
      return (spreadingFactor * (bandwidth*4 / std::pow(2, spreadingFactor)) * (1.0f/(codingRate+4.0f)));
    }
};

/**
 * Class representing the MAC layer of a Class A LoRaWAN device.
 */
class ClassAEndDeviceLorawanMac : public EndDeviceLorawanMac
{
public:
  static TypeId GetTypeId (void);

  ClassAEndDeviceLorawanMac ();
  virtual ~ClassAEndDeviceLorawanMac ();

  //research 
  void recievedAck();

  /////////////////////
  // Sending methods //
  /////////////////////

  /**
  * Add headers and send a packet with the sending function of the physical layer.
  *
  * \param packet the packet to send
  */
  virtual void SendToPhy (Ptr<Packet> packet);

  //////////////////////////
  //  Receiving methods   //
  //////////////////////////

  /**
   * Receive a packet.
   *
   * This method is typically registered as a callback in the underlying PHY
   * layer so that it's called when a packet is going up the stack.
   *
   * \param packet the received packet.
   */
  virtual void Receive (Ptr<Packet const> packet);

  virtual void FailedReception (Ptr<Packet const> packet);

  /**
   * Perform the actions that are required after a packet send.
   *
   * This function handles opening of the first receive window.
   */
  virtual void TxFinished (Ptr<const Packet> packet);

  /**
   * Perform operations needed to open the first receive window.
   */
  void OpenFirstReceiveWindow (void);

  /**
   * Perform operations needed to open the second receive window.
   */
  void OpenSecondReceiveWindow (void);

  /**
   * Perform operations needed to close the first receive window.
   */
  void CloseFirstReceiveWindow (void);

  /**
   * Perform operations needed to close the second receive window.
   */
  void CloseSecondReceiveWindow (void);

  /////////////////////////
  // Getters and Setters //
  /////////////////////////

  /**
   * Find the minimum waiting time before the next possible transmission based
   * on End Device's Class Type.
   *
   * \param waitingTime The minimum waiting time that has to be respected,
   * irrespective of the class (e.g., because of duty cycle limitations).
   */
  virtual Time GetNextClassTransmissionDelay (Time waitingTime);

  /**
   * Get the Data Rate that will be used in the first receive window.
   *
   * \return The Data Rate
   */
  uint8_t GetFirstReceiveWindowDataRate (void);

  /**
   * Set the Data Rate to be used in the second receive window.
   *
   * \param dataRate The Data Rate.
   */
  void SetSecondReceiveWindowDataRate (uint8_t dataRate);

  /**
   * Get the Data Rate that will be used in the second receive window.
   *
   * \return The Data Rate
   */
  uint8_t GetSecondReceiveWindowDataRate (void);

  /**
   * Set the frequency that will be used for the second receive window.
   *
   * \param frequencyMHz the Frequency.
   */
  void SetSecondReceiveWindowFrequency (double frequencyMHz);

  /**
   * Get the frequency that is used for the second receive window.
   *
   * @return The frequency, in MHz
   */
  double GetSecondReceiveWindowFrequency (void);

  //CUSTOM MACHINE LEARNING MEMBERS:

  bool UseMachineLearningOptimization;

  // Called when an acknowledgement is requested but none is recieved.
  void AckNotRecieved (void);

  int getIndexOfFittestIndividual(void);

  void advanceGeneration(void);
  /////////////////////////
  // MAC command methods //
  /////////////////////////

  /**
   * Perform the actions that need to be taken when receiving a RxParamSetupReq
   * command based on the Device's Class Type.
   *
   * \param rxParamSetupReq The Parameter Setup Request, which contains:
   *                            - The offset to set.
   *                            - The data rate to use for the second receive window.
   *                            - The frequency to use for the second receive window.
   */
  virtual void OnRxClassParamSetupReq (Ptr<RxParamSetupReq> rxParamSetupReq);

private:

  /**
   * The interval between when a packet is done sending and when the first
   * receive window is opened.
   */
  Time m_receiveDelay1;

  /**
   * The interval between when a packet is done sending and when the second
   * receive window is opened.
   */
  Time m_receiveDelay2;

  /**
   * The event of the closing the first receive window.
   *
   * This Event will be canceled if there's a successful reception of a packet.
   */
  EventId m_closeFirstWindow;

  /**
   * The event of the closing the second receive window.
   *
   * This Event will be canceled if there's a successful reception of a packet.
   */
  EventId m_closeSecondWindow;

  /**
   * The event of the second receive window opening.
   *
   * This Event is used to cancel the second window in case the first one is
   * successful.
   */
  EventId m_secondReceiveWindow;

  /**
   * The frequency to listen on for the second receive window.
   */
  double m_secondReceiveWindowFrequency;

  /**
   * The Data Rate to listen for during the second downlink transmission.
   */
  uint8_t m_secondReceiveWindowDataRate;

  /**
   * The RX1DROffset parameter value
   */
  uint8_t m_rx1DrOffset;

  //The number of Unconfirmed messages we sent out.
  uint8_t m_unconfirmedTransmissions;
  uint8_t m_ga_currentIndex;

  bool useGeneticParamaterSelection;

  TXParameterIndividual* population[16];
}; /* ClassAEndDeviceLorawanMac */
} /* namespace lorawan */
} /* namespace ns3 */
#endif /* CLASS_A_END_DEVICE_LORAWAN_MAC_H */
