import pyarrow.parquet as pq
import pyarrow as pa # pip install pyarrow==0.7.1
print("pyarrow imported")
import ROOT
print("ROOT imported")
import numpy as np
import glob, os
from skimage.measure import block_reduce # pip install scikit-image
from numpy.lib.stride_tricks import as_strided
print("Numpy related packages imported")

import argparse

print("Successfully installed packages")

parser = argparse.ArgumentParser(description='Process some integers.')
parser.add_argument('-i', '--infile', default='output_qqgg.root', type=str, help='Input root file.')
parser.add_argument('-o', '--outdir', default='.', type=str, help='Output pq file dir.')
parser.add_argument('-d', '--decay', default='test', type=str, help='Decay name.')
parser.add_argument('-n', '--idx', default=0, type=int, help='Input root file index.')
args = parser.parse_args()

print("Parsed arguments")

def upsample_array(x, b0, b1):

    r, c = x.shape                                    # number of rows/columns
    rs, cs = x.strides                                # row/column strides
    x = as_strided(x, (r, b0, c, b1), (rs, 0, cs, 0)) # view as a larger 4D array

    return x.reshape(r*b0, c*b1)/(b0*b1)              # create new 2D array with same total occupancy 

def resample_EE(imgECAL, factor=2):

    # EE-
    imgEEm = imgECAL[:140-85] # EE- in the first 55 rows
    imgEEm = np.pad(imgEEm, ((1,0),(0,0)), 'constant', constant_values=0) # for even downsampling, zero pad 55 -> 56
    imgEEm_dn = block_reduce(imgEEm, block_size=(factor, factor), func=np.sum) # downsample by summing over [factor, factor] window
    imgEEm_dn_up = upsample_array(imgEEm_dn, factor, factor)/(factor*factor) # upsample will use same values so need to correct scale by factor**2
    imgECAL[:140-85] = imgEEm_dn_up[1:] ## replace the old EE- rows

    # EE+
    imgEEp = imgECAL[140+85:] # EE+ in the last 55 rows
    imgEEp = np.pad(imgEEp, ((0,1),(0,0)), 'constant', constant_values=0) # for even downsampling, zero pad 55 -> 56
    imgEEp_dn = block_reduce(imgEEp, block_size=(factor, factor), func=np.sum) # downsample by summing over [factor, factor] window
    imgEEp_dn_up = upsample_array(imgEEp_dn, factor, factor)/(factor*factor) # upsample will use same values so need to correct scale by factor*factor
    imgECAL[140+85:] = imgEEp_dn_up[:-1] # replace the old EE+ rows

    return imgECAL

def crop_jet(imgECAL, iphi, ieta, jet_shape=125):

    # NOTE: jet_shape here should correspond to the one used in RHAnalyzer
    off = jet_shape//2
    iphi = int(iphi*5 + 2) # 5 EB xtals per HB tower
    ieta = int(ieta*5 + 2) # 5 EB xtals per HB tower

    # Wrap-around on left side
    if iphi < off:
        diff = off-iphi
        img_crop = np.concatenate((imgECAL[:,ieta-off:ieta+off+1,-diff:],
                                   imgECAL[:,ieta-off:ieta+off+1,:iphi+off+1]), axis=-1)
    # Wrap-around on right side
    elif 360-iphi < off:
        diff = off - (360-iphi)
        img_crop = np.concatenate((imgECAL[:,ieta-off:ieta+off+1,iphi-off:],
                                   imgECAL[:,ieta-off:ieta+off+1,:diff+1]), axis=-1)
    # Nominal case
    else:
        img_crop = imgECAL[:,ieta-off:ieta+off+1,iphi-off:iphi+off+1]

    return img_crop

rhTreeStr = args.infile
print("Loaded file") 
rhTree = ROOT.TChain("recHitAnalyzer/RHTree") # change fevt/RHTree to just RHTree
print("Created tree")
rhTree.Add(rhTreeStr)
print("Added String")
nEvts = rhTree.GetEntries()
assert nEvts > 0
print " >> Input file:",rhTreeStr
print " >> nEvts:",nEvts
outStr = '%s/%s.parquet.%d'%(args.outdir, args.decay, args.idx) 
print " >> Output file:",outStr

##### MAIN #####

# Event range to process
iEvtStart = 0
#iEvtEnd   = 10
iEvtEnd   = nEvts 
assert iEvtEnd <= nEvts
print " >> Processing entries: [",iEvtStart,"->",iEvtEnd,")"

nJets = 0
data = {} # Arrays to be written to parquet should be saved to data dict
sw = ROOT.TStopwatch()
sw.Start()
for iEvt in range(iEvtStart,iEvtEnd):

    # Initialize event
    rhTree.GetEntry(iEvt)

    if iEvt % 10000 == 0:
        print " .. Processing entry",iEvt

    ECAL_energy = np.array(rhTree.ECAL_energy).reshape(280,360)
    ECAL_energy = resample_EE(ECAL_energy)
    HBHE_energy = np.array(rhTree.HBHE_energy).reshape(56,72)
    HBHE_energy = upsample_array(HBHE_energy, 5, 5) # (280, 360)
    TracksAtECAL_pt = np.array(rhTree.ECAL_tracksPt).reshape(280,360)
    data['X_CMSII'] = np.stack([TracksAtECAL_pt, ECAL_energy, HBHE_energy], axis=0) # (3, 280, 360)

    # Jet attributes 
    # ys = rhTree.jetIsQuark # Doesn't exist
    pts = rhTree.jetPt

    #add values
    phis = rhTree.jetPhi
    etas = rhTree.jetEta

    iphis = rhTree.jetSeed_iphi
    ietas = rhTree.jetSeed_ieta
    jet_truthLabel = rhTree.jet_truthLabel
    DM = rhTree.jet_truthDM #add DM
    # pdgIds = rhTree.jetPdgIds # Doesn't exist
    njets = len(pts) #change to pt since y doesn't exist

    for i in range(njets):

        # data['y'] = ys[i] # doesnt exist
        data['pt'] = pts[i]
        data['iphi'] = iphis[i]
        data['ieta'] = ietas[i]
        data['phi'] = phis[i]
        data['eta'] = etas[i]
        data['DM'] = DM[i]
        data["truth"] = jet_truthLabel[i]
        # data['pdgId'] = pdgIds[i] # doesn't exist
        data['X_jet'] = crop_jet(data['X_CMSII'], data['iphi'], data['ieta']) # (3, 125, 125)

        # Create pyarrow.Table

        pqdata = [pa.array([d]) if (np.isscalar(d) or type(d) == list) else pa.array([d.tolist()]) for d in data.values()]

        table = pa.Table.from_arrays(pqdata, data.keys())

        if nJets == 0:
            writer = pq.ParquetWriter(outStr, table.schema, compression='snappy')

        writer.write_table(table)

        nJets += 1

writer.close()
print " >> nJets:",nJets
print " >> Real time:",sw.RealTime()/60.,"minutes"
print " >> CPU time: ",sw.CpuTime() /60.,"minutes"
print "========================================================"

# Verify output file
pqIn = pq.ParquetFile(outStr)
print(pqIn.metadata)
print(pqIn.schema)
# X = pqIn.read_row_group(0, columns=['y','pt','iphi','ieta','pdgId']).to_pydict()
X = pqIn.read_row_group(0, columns=['pt','iphi','ieta']).to_pydict() # RM vars
print(X)
#X = pqIn.read_row_group(0, columns=['X_jet.list.item.list.item.list.item']).to_pydict()['X_jet'] # read row-by-row 
#X = pqIn.read(['X_jet.list.item.list.item.list.item', 'y']).to_pydict()['X_jet'] # read entire column(s)
#X = np.float32(X)
