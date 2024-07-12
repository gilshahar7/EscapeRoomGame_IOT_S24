import React, { useRef, useState, useEffect } from 'react'
import { Canvas } from '@react-three/fiber'
import { OrbitControls, Html, PerspectiveCamera} from '@react-three/drei'

const DEPTH_MULTIPLIER = 2.5
const SCALE_FACTOR = 1.5; // Change this value to spread the dots more or less

const fixedPositions = [
  { position: [-0.8 * SCALE_FACTOR, 0.8 * SCALE_FACTOR, 0.1 * DEPTH_MULTIPLIER], name: "Ree" },
  { position: [-0.5 * SCALE_FACTOR, 0.9 * SCALE_FACTOR, 0.2 * DEPTH_MULTIPLIER], name: "Wonn" },
  { position: [-0.2 * SCALE_FACTOR, 0.9 * SCALE_FACTOR, 0.3 * DEPTH_MULTIPLIER], name: "Sikstee" },
  { position: [0.3 * SCALE_FACTOR, 0.9 * SCALE_FACTOR, 0.1 * DEPTH_MULTIPLIER], name: "AT-2" },
  { position: [0.7 * SCALE_FACTOR, 0.9 * SCALE_FACTOR, 0.2 * DEPTH_MULTIPLIER], name: "Hun" },
  { position: [0.8 * SCALE_FACTOR, 0.5 * SCALE_FACTOR, 0.3 * DEPTH_MULTIPLIER], name: "Phorr" },
  { position: [-0.9 * SCALE_FACTOR, 0.6 * SCALE_FACTOR, 0.2 * DEPTH_MULTIPLIER], name: "Sev" },
  { position: [0.1 * SCALE_FACTOR, 0.7 * SCALE_FACTOR, 0.1 * DEPTH_MULTIPLIER], name: "Tahun" },
  { position: [-0.6 * SCALE_FACTOR, 0.2 * SCALE_FACTOR, 0.3 * DEPTH_MULTIPLIER], name: "Toonyne" },
  { position: [0 * SCALE_FACTOR, 0.2 * SCALE_FACTOR, 0.2 * DEPTH_MULTIPLIER], name: "Fonata" },
  { position: [0.4 * SCALE_FACTOR, 0.2 * SCALE_FACTOR, 0.1 * DEPTH_MULTIPLIER], name: "Enti" },
  { position: [0.7 * SCALE_FACTOR, 0.2 * SCALE_FACTOR, 0.3 * DEPTH_MULTIPLIER], name: "Tiwon" },
  { position: [-0.7 * SCALE_FACTOR, -0.1 * SCALE_FACTOR, 0.2 * DEPTH_MULTIPLIER], name: "Aytee" },
  { position: [-0.2 * SCALE_FACTOR, -0.2 * SCALE_FACTOR, 0.1 * DEPTH_MULTIPLIER], name: "NT-9" },
  { position: [0.1 * SCALE_FACTOR, -0.2 * SCALE_FACTOR, 0.3 * DEPTH_MULTIPLIER], name: "Minuss" },
  { position: [0.5 * SCALE_FACTOR, -0.1 * SCALE_FACTOR, 0.2 * DEPTH_MULTIPLIER], name: "Forti" },
  { position: [0.8 * SCALE_FACTOR, -0.3 * SCALE_FACTOR, 0.1 * DEPTH_MULTIPLIER], name: "Dreddand" },
  { position: [-0.9 * SCALE_FACTOR, -0.8 * SCALE_FACTOR, 0.3 * DEPTH_MULTIPLIER], name: "Thurt" },
  { position: [-0.3 * SCALE_FACTOR, -0.6 * SCALE_FACTOR, 0.2 * DEPTH_MULTIPLIER], name: "Phives" },
  { position: [0.2 * SCALE_FACTOR, -0.5 * SCALE_FACTOR, 0.1 * DEPTH_MULTIPLIER], name: "Nyne" },
  { position: [0.6 * SCALE_FACTOR, -0.7 * SCALE_FACTOR, 0.3 * DEPTH_MULTIPLIER], name: "Xiero" }
];


function Dots() {
  const groupRef = useRef()

  return (
    <group ref={groupRef} rotation={[-Math.PI / 4, 0, 0]}>
      {fixedPositions.map(({ position, name }, i) => {
        const [x, y, z] = position;
        return (
          <React.Fragment key={i}>
            <mesh position={[x, y, z]}>
              <sphereGeometry args={[0.03, 32, 32]} />
              <meshBasicMaterial color="lightyellow" />
            </mesh>
            <Html
              position={[x, y, z + 0.1]} // Increased z offset to move label further from dot
              style={{
                color: 'white',
                fontSize: '16px', // Increased from 12px to 16px
                fontWeight: 'bold',
                whiteSpace: 'nowrap',
                userSelect: 'none',
                textShadow: '2px 2px 4px rgba(0,0,0,0.8)',
              }}
            >
              <div>{name}</div>
            </Html>
          </React.Fragment>
        );
      })}
    </group>
  )
}

function App() {
  return (
    <div style={{ width: '100vw', height: '100vh', backgroundColor: '#00008B' }}>
      <Canvas
        camera={{ 
          position: [0, 2, 2], 
          fov: 60, 
          near: 0.1, 
          far: 1000,
          up: [0, 0, 1] // This sets the camera's up vector to the z-axis
        }}
        style={{ background: 'black' }}
      >
        <PerspectiveCamera 
          makeDefault 
          position={[0, -2, 2]} 
          fov={60}
          near={0.1}
          far={1000}
          up={[0, 0, 1]}
        />
        <OrbitControls 
          enableZoom={true}
          enablePan={false}
          enableRotate={true}
          minPolarAngle={-Infinity}
          maxPolarAngle={Infinity}
          minAzimuthAngle={-Infinity}
          maxAzimuthAngle={Infinity}
          autoRotate={false}
          enableDamping={true}
          dampingFactor={0.05}
        />
        <ambientLight intensity={0.5} />
        <pointLight position={[10, 10, 10]} />
        <Dots />
      </Canvas>
    </div>
  )
}

export default App