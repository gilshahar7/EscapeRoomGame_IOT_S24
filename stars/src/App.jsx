import React, { useRef, useState, useMemo } from 'react';
import { Canvas, useFrame } from '@react-three/fiber';
import { OrbitControls, Text, Line } from '@react-three/drei';
import * as THREE from 'three';
import OrientationNotification from './OrientationNotification';

const SCALE_FACTOR = 2; // Adjust this factor to control the spread
const X_STRETCH_FACTOR = 2; // Factor to stretch x-coordinates

const fixedPositions = [
  { name: "Ree", position: [-0.8 * SCALE_FACTOR * X_STRETCH_FACTOR, 0.8 * SCALE_FACTOR, -0.5 * SCALE_FACTOR] },
  { name: "Sev", position: [-0.5 * SCALE_FACTOR * X_STRETCH_FACTOR, 0.9 * SCALE_FACTOR, 0.3 * SCALE_FACTOR] },
  { name: "Enti", position: [-0.2 * SCALE_FACTOR * X_STRETCH_FACTOR, 0.9 * SCALE_FACTOR, 0.7 * SCALE_FACTOR] },
  { name: "Tahun", position: [0.3 * SCALE_FACTOR * X_STRETCH_FACTOR, 0.9 * SCALE_FACTOR, -0.3 * SCALE_FACTOR] },
  { name: "Thurt", position: [0.7 * SCALE_FACTOR * X_STRETCH_FACTOR, 0.9 * SCALE_FACTOR, 0.8 * SCALE_FACTOR] },
  { name: "Phorr", position: [0.8 * SCALE_FACTOR * X_STRETCH_FACTOR, 0.5 * SCALE_FACTOR, -0.6 * SCALE_FACTOR] },
  { name: "Wonn", position: [-0.9 * SCALE_FACTOR * X_STRETCH_FACTOR, 0.6 * SCALE_FACTOR, 0.2 * SCALE_FACTOR] },
  { name: "AT-2", position: [0.1 * SCALE_FACTOR * X_STRETCH_FACTOR, 0.7 * SCALE_FACTOR, -0.1 * SCALE_FACTOR] },
  { name: "Toonyne", position: [-0.6 * SCALE_FACTOR * X_STRETCH_FACTOR, 0.2 * SCALE_FACTOR, 0.9 * SCALE_FACTOR] },
  { name: "Kpyr", position: [0 * SCALE_FACTOR * X_STRETCH_FACTOR, 0.2 * SCALE_FACTOR, -0.8 * SCALE_FACTOR] },
  { name: "Sirius", position: [0.4 * SCALE_FACTOR * X_STRETCH_FACTOR, 0.1 * SCALE_FACTOR, 0.5 * SCALE_FACTOR] },
  { name: "Vega", position: [0.8 * SCALE_FACTOR * X_STRETCH_FACTOR, 0.2 * SCALE_FACTOR, -0.4 * SCALE_FACTOR] },
  { name: "Aytee", position: [-0.7 * SCALE_FACTOR * X_STRETCH_FACTOR, -0.1 * SCALE_FACTOR, 0.6 * SCALE_FACTOR] },
  { name: "NT-9", position: [-0.2 * SCALE_FACTOR * X_STRETCH_FACTOR, -0.1 * SCALE_FACTOR, -0.2 * SCALE_FACTOR] },
  { name: "Dreddand", position: [0.1 * SCALE_FACTOR * X_STRETCH_FACTOR, -0.2 * SCALE_FACTOR, 0.4 * SCALE_FACTOR] },
  { name: "Forti", position: [0.5 * SCALE_FACTOR * X_STRETCH_FACTOR, -0.1 * SCALE_FACTOR, -0.9 * SCALE_FACTOR] },
  { name: "Minuss", position: [0.8 * SCALE_FACTOR * X_STRETCH_FACTOR, -0.3 * SCALE_FACTOR, 0.1 * SCALE_FACTOR] },
  { name: "Hun", position: [-0.9 * SCALE_FACTOR * X_STRETCH_FACTOR, -0.8 * SCALE_FACTOR, -0.7 * SCALE_FACTOR] },
  // { name: "Phives", position: [-0.3 * SCALE_FACTOR * X_STRETCH_FACTOR, -0.6 * SCALE_FACTOR, 0 * SCALE_FACTOR] },
  { name: "Nyne", position: [0.2 * SCALE_FACTOR * X_STRETCH_FACTOR, -0.5 * SCALE_FACTOR, 0.2 * SCALE_FACTOR] },
  { name: "Xiero", position: [0.6 * SCALE_FACTOR * X_STRETCH_FACTOR, -0.7 * SCALE_FACTOR, -0.5 * SCALE_FACTOR] },
  { name: "Sikstee", position: [-0.4 * SCALE_FACTOR * X_STRETCH_FACTOR, 0.5 * SCALE_FACTOR, -0.4 * SCALE_FACTOR] },
  { name: "Betelgeuse", position: [0.5 * SCALE_FACTOR * X_STRETCH_FACTOR, -0.4 * SCALE_FACTOR, 0.6 * SCALE_FACTOR] },
  { name: "Tiwon", position: [0.3 * SCALE_FACTOR * X_STRETCH_FACTOR, 0.6 * SCALE_FACTOR, 0.4 * SCALE_FACTOR] },
  { name: "Altair", position: [-0.6 * SCALE_FACTOR * X_STRETCH_FACTOR, -0.3 * SCALE_FACTOR, -0.3 * SCALE_FACTOR] },
  { name: "Procyon", position: [-0.1 * SCALE_FACTOR * X_STRETCH_FACTOR, 0.4 * SCALE_FACTOR, -0.7 * SCALE_FACTOR] },
];

const connections = [
  'Xiero_Phorr_Hun_Dreddand_Nyne',
  'Sev_Enti_Thurt_Tiwon',
  'Aytee_Toonyne_NT-9',
  'AT-2_Sikstee_Wonn'
];

function ConnectingLine({ start, end }) {
  return (
    <Line
      points={[start, end]}
      color="white"
      lineWidth={1}
      dashed
      dashSize={0.05}
      gapSize={0.15}
    />
  );
}

function Dots() {
  const groupRef = useRef();
  const [showLabels, setShowLabels] = useState(false);
  const [initialAngleSet, setInitialAngleSet] = useState(false);

  // Parse connections and prepare lines
  const lines = useMemo(() => {
    const parsedLines = [];

    connections.forEach(connection => {
      const dots = connection.split('_');
      for (let i = 0; i < dots.length - 1; i++) {
        const startDot = fixedPositions.find(dot => dot.name === dots[i]);
        const endDot = fixedPositions.find(dot => dot.name === dots[i + 1]);
        if (startDot && endDot) {
          parsedLines.push({ start: startDot.position, end: endDot.position });
        }
      }
    });

    return parsedLines;
  }, []);

  useFrame(({ camera }) => {
    if (!initialAngleSet) {
      // Set a random initial camera position
      camera.position.set(
        Math.random() * 5,
        Math.random() * 5,
        Math.random() * 5
      );
      setInitialAngleSet(true);
    }

    if (groupRef.current) {
      // Calculate the vector from the group to the camera
      const groupToCamera = new THREE.Vector3().subVectors(camera.position, groupRef.current.position);

      // Convert camera position to spherical coordinates relative to the group's position
      const spherical = new THREE.Spherical().setFromVector3(groupToCamera);

      // Convert angles to degrees
      const azimuthalAngle = (spherical.theta * 180) / Math.PI;
      const polarAngle = (spherical.phi * 180) / Math.PI;

      // Define target angles and tolerances
      const targetAzimuthal = 0; // degrees
      const targetPolar = 90; // degrees
      const azimuthalTolerance = 15; // degrees
      const polarTolerance = 15; // degrees

      // Condition to toggle labels
      if (
        azimuthalAngle > targetAzimuthal - azimuthalTolerance &&
        azimuthalAngle < targetAzimuthal + azimuthalTolerance &&
        polarAngle > targetPolar - polarTolerance &&
        polarAngle < targetPolar + polarTolerance
      ) {
        setShowLabels(true);

        // Calculate target rotation using quaternion
        const targetQuaternion = new THREE.Quaternion();
        targetQuaternion.setFromUnitVectors(
          new THREE.Vector3(0, 0, 1),
          groupToCamera.clone().normalize()
        );

        // Smooth interpolation using slerp
        const delta = 0.05; // Adjust this value for smoothness
        groupRef.current.quaternion.slerp(targetQuaternion, delta);
      } else {
        setShowLabels(false);
      }
    }
  });

  return (
    <group ref={groupRef}>
      {fixedPositions.map(({ position, name }, i) => (
        <React.Fragment key={i}>
          <mesh position={position}>
            <sphereGeometry args={[0.1, 32, 32]} />
            <meshBasicMaterial color="#ffff6e" />
          </mesh>
          {showLabels && (
            <Text
              position={[position[0], position[1] + 0.2, position[2]]}
              color="white"
              fontSize={0.15}
              maxWidth={2}
              textAlign="center"
              anchorY="bottom"
            >
              {name}
            </Text>
          )}
        </React.Fragment>
      ))}
      {lines.map((line, index) => (
        <ConnectingLine key={index} start={line.start} end={line.end} />
      ))}
    </group>
  );
}

function App() {
  return (
    <div style={{ width: '100vw', height: '100vh', backgroundColor: '#000' }}>
      <OrientationNotification />
      <Canvas camera={{ position: [0, 0, 5] }}>
        <OrbitControls />
        <Dots />
      </Canvas>
    </div>
  );
}

export default App;
