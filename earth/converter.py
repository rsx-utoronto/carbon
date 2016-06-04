# converts files from 46 38.242 (46 degrees, 38 minutes) simple to decimals 46.6...
import sys 

indir = './terrain.raw'
outdir = './terrain.waypoints'

print(sys.argv)

if len(sys.argv) >= 2: 
  indir = sys.argv[1]
  outdir = sys.argv[1].replace('.raw', '.waypoints') 

if len(sys.argv) >= 3:
  outdir = sys.argv[2]

with open(indir, 'r') as f: 
  with open(outdir, 'w') as out:
    for line in f:
      name, longitude_degrees, longitude_minutes, latitude_degrees, latitude_minutes = line.split(',') 
    
      longitude = float(longitude_degrees) + float(longitude_minutes) / 60
      latitude = float(latitude_degrees) + float(latitude_minutes) / 60  
      out.write("{}, {}, {}\n".format(name, longitude, latitude))
  out.close() 
f.close() 
