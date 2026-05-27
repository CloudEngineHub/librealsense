import { useMemo } from 'react'
import { Canvas } from '@react-three/fiber'
import { OrbitControls, PerspectiveCamera } from '@react-three/drei'
import * as THREE from 'three'
import { useAppStore } from '../store'

export function PointCloudViewer() {
  const { pointCloudVertices, isStreaming } = useAppStore()

  return (
    <div className="h-full flex flex-col">
      {/* Controls */}
      <div className="flex items-center justify-end gap-4 p-2 bg-gray-800 rounded-t-lg">
        <button
          onClick={() => {
            if (pointCloudVertices) {
              exportToPLY(pointCloudVertices)
            }
          }}
          disabled={!pointCloudVertices}
          className="control-button-secondary text-sm py-1"
        >
          Export PLY
        </button>
      </div>

      {/* 3D Canvas */}
      <div className="flex-1 bg-black rounded-b-lg overflow-hidden">
        {pointCloudVertices ? (
          <Canvas>
            <PerspectiveCamera makeDefault position={[0, 0.3, 0.3]} />
            <OrbitControls enablePan enableZoom enableRotate target={[0, 0, -1]} />
            <ambientLight intensity={0.5} />
            <PointCloud vertices={pointCloudVertices} />
            <Axes />
            <gridHelper args={[10, 10, '#444', '#333']} />
          </Canvas>
        ) : (
          <div className="h-full flex items-center justify-center text-gray-500">
            <div className="text-center">
              <svg
                className="w-16 h-16 mx-auto mb-4 opacity-50"
                fill="none"
                stroke="currentColor"
                viewBox="0 0 24 24"
              >
                <path
                  strokeLinecap="round"
                  strokeLinejoin="round"
                  strokeWidth={1}
                  d="M14 10l-2 1m0 0l-2-1m2 1v2.5M20 7l-2 1m2-1l-2-1m2 1v2.5M14 4l-2-1-2 1M4 7l2-1M4 7l2 1M4 7v2.5M12 21l-2-1m2 1l2-1m-2 1v-2.5M6 18l-2-1v-2.5M18 18l2-1v-2.5"
                />
              </svg>
              <p className="text-lg">3D Point Cloud View</p>
              <p className="text-sm mt-1">
                {isStreaming
                  ? 'Waiting for point cloud data…'
                  : 'Start streaming depth to see points'}
              </p>
            </div>
          </div>
        )}
      </div>
    </div>
  )
}

interface PointCloudProps {
  vertices: Float32Array
}

function PointCloud({ vertices }: PointCloudProps) {
  const geometry = useMemo(() => {
    const geo = new THREE.BufferGeometry()
    const n = vertices.length
    const positions = new Float32Array(n)
    const colors = new Float32Array(n)
    const BINS = 64

    let zMin = Infinity
    let zMax = -Infinity
    for (let i = 2; i < n; i += 3) {
      const z = vertices[i]
      if (z < zMin) zMin = z
      if (z > zMax) zMax = z
    }

    // Histogram + CDF (matches rs.colorizer histogram_eq default) so a few far-z
    // outliers don't crush the visible scene into a single color band.
    const cdf = new Float32Array(BINS)
    const binScale = isFinite(zMin) && zMax > zMin ? BINS / (zMax - zMin) : 0
    if (binScale > 0) {
      const hist = new Uint32Array(BINS)
      for (let i = 2; i < n; i += 3) {
        let bin = Math.floor((vertices[i] - zMin) * binScale)
        if (bin >= BINS) bin = BINS - 1
        else if (bin < 0) bin = 0
        hist[bin]++
      }
      let acc = 0
      for (let b = 0; b < BINS; b++) {
        acc += hist[b]
        cdf[b] = acc
      }
      if (acc > 0) for (let b = 0; b < BINS; b++) cdf[b] /= acc
    }

    for (let i = 0; i < n; i += 3) {
      // RealSense: x-right, y-down, z-forward (meters)
      // Three.js:  x-right, y-up,   z-toward-camera
      positions[i] = vertices[i]
      positions[i + 1] = -vertices[i + 1]
      positions[i + 2] = -vertices[i + 2]

      let t = 0
      if (binScale > 0) {
        let bin = Math.floor((vertices[i + 2] - zMin) * binScale)
        if (bin >= BINS) bin = BINS - 1
        else if (bin < 0) bin = 0
        t = cdf[bin]
      }
      const c = jetColor(t)
      colors[i] = c[0]
      colors[i + 1] = c[1]
      colors[i + 2] = c[2]
    }

    geo.setAttribute('position', new THREE.BufferAttribute(positions, 3))
    geo.setAttribute('color', new THREE.BufferAttribute(colors, 3))
    geo.computeBoundingSphere()
    return geo
  }, [vertices])

  return (
    <points geometry={geometry}>
      <pointsMaterial size={0.005} vertexColors sizeAttenuation />
    </points>
  )
}

// Jet colormap (blue → cyan → green → yellow → red) — matches librealsense rs.colorizer default.
function jetColor(t: number): [number, number, number] {
  const v = Math.min(Math.max(t, 0), 1)
  if (v < 0.125) return [0, 0, 0.5 + v * 4]
  if (v < 0.375) return [0, (v - 0.125) * 4, 1]
  if (v < 0.625) return [(v - 0.375) * 4, 1, 1 - (v - 0.375) * 4]
  if (v < 0.875) return [1, 1 - (v - 0.625) * 4, 0]
  return [1 - (v - 0.875) * 4, 0, 0]
}

function Axes() {
  return (
    <group>
      {/* X axis - Red */}
      <line>
        <bufferGeometry>
          <bufferAttribute
            attach="attributes-position"
            args={[new Float32Array([0, 0, 0, 1, 0, 0]), 3]}
          />
        </bufferGeometry>
        <lineBasicMaterial color="red" />
      </line>
      {/* Y axis - Green */}
      <line>
        <bufferGeometry>
          <bufferAttribute
            attach="attributes-position"
            args={[new Float32Array([0, 0, 0, 0, 1, 0]), 3]}
          />
        </bufferGeometry>
        <lineBasicMaterial color="green" />
      </line>
      {/* Z axis - Blue */}
      <line>
        <bufferGeometry>
          <bufferAttribute
            attach="attributes-position"
            args={[new Float32Array([0, 0, 0, 0, 0, 1]), 3]}
          />
        </bufferGeometry>
        <lineBasicMaterial color="blue" />
      </line>
    </group>
  )
}

function exportToPLY(vertices: Float32Array) {
  const numPoints = vertices.length / 3
  
  let plyContent = `ply
format ascii 1.0
element vertex ${numPoints}
property float x
property float y
property float z
end_header
`

  for (let i = 0; i < vertices.length; i += 3) {
    plyContent += `${vertices[i]} ${vertices[i + 1]} ${vertices[i + 2]}\n`
  }

  // Create and download file
  const blob = new Blob([plyContent], { type: 'text/plain' })
  const url = URL.createObjectURL(blob)
  const link = document.createElement('a')
  link.href = url
  link.download = `pointcloud_${Date.now()}.ply`
  document.body.appendChild(link)
  link.click()
  document.body.removeChild(link)
  URL.revokeObjectURL(url)
}
