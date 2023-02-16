/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2018-2022 The IoD_Sim Authors.
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
 */
#ifndef DRONE_H
#define DRONE_H

#define GRAVITY 9.81

#include <ns3/vector.h>
#include <ns3/node.h>
#include <ns3/callback.h>
#include <ns3/ptr.h>
#include <ns3/net-device.h>
#include "drone-peripheral-container.h"
//my edit
#include <ns3/energy-source.h>

namespace ns3 {

class DronePeripheralContainer;

/**
 * \ingroup network
 *
 * This class represents a drone in a network.
 * The Drone is characterized by a set of mechanical properties and
 * a set of peripherals that are attached to it.
 */
class Drone : public Node
{
public:
  /**
   * \brief Get the type ID.
   *
   * \returns the object TypeId
   */
  static TypeId GetTypeId (void);

  Drone ();

  /**
   * \brief Sets the mass of the drone.
   *
   * \param mass Mass of the drone.
   */
  void setMass (double mass);

  /**
   * \brief Sets the area of the rotor disk.
   *
   * \param area Area of the rotor disk.
   */
  void setArea (double area);

  /**
   * \brief Sets the drag coefficient.
   *
   * \param coefficient Drag coefficient.
   */
  void setDragCoefficient (double coefficient);

  /**
   * \brief Sets the energy source.
   *
   * \param energySource EnergySource of drone.
   */
  void setEnergySource (Ptr<EnergySource> energySource);

  /**
   * \brief Returns the mass of the drone.
   *
   * \returns Mass of the drone.
   */
  double getMass () const;

  /**
   * \brief Returns the weight force applied to the drone.
   *
   * \returns Weight force.
   */
  double getWeight () const;

  /**
   * \brief Returns the area of the rotor disk.
   *
   * \returns Area of the rotor disk.
   */
  double getArea () const;

  /**
   * \brief Returns the drag coefficient.
   *
   * \returns Drag coefficient.
   */
  double getDragCoefficient () const;

  /**
   * \brief Returns the peripheral container of the drone.
   *
   * \returns peripheral Pointer to a DronePeripheralContainer.
   */
  Ptr<DronePeripheralContainer> getPeripherals ();

  /**
   * \brief Returns the energy source of the drone.
   *
   * \returns energy source Pointer to a EnergyModel.
   */
  Ptr<EnergySource> getEnergySource ();

protected:
  virtual void DoDispose (void);
  virtual void DoInitialize (void);

private:
  double m_mass; //!< Mass of the drone
  double m_diskArea; //!< Area of the rotor disk
  double m_weightForce; //!< Weight force, equals to m*g
  double m_airDensity; //!< Air density
  double m_dragCoefficient; //!< Drag Coefficient
  Ptr<DronePeripheralContainer> m_peripheralContainer;
  // my edit
  Ptr<EnergySource> m_energySource;
};

} //namespace ns3

#endif /* DRONE_H */
