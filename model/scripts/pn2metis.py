import pandas as pd
import glob, os, sys
from collections import defaultdict
import networkx as nx
import csv
from tqdm import tqdm

def load_places(places_path):
    places = pd.read_csv(places_path)
    places.reset_index()
    # add places metis idx
    places['vid'] = places.index + 1
    places.set_index('numeric_id', inplace=True)
    return places

def create_place_id_maps(places_path):
    nid_to_i = {}
    i_to_nid = {}

    with open(places_path) as f_in:
        reader = csv.reader(f_in)
        next(reader)
        for i, row in enumerate(reader):
            # float first to handle sci notation
            numeric_id = int(float(row[0]))
            idx = i + 1
            nid_to_i[numeric_id] = idx
            i_to_nid[idx] = numeric_id

    return (nid_to_i, i_to_nid)


def add_edge(g, v1, v2):
    if g.has_edge(v1, v2):
        g[v1][v2]['weight'] += 1
    else:
        g.add_edge(v1, v2, weight = 1)

def update_vertex_weight(g, v):
    if g.has_node(v):
        node = g.node[v]
        if 'weight' in node:
            node['weight'] += 1
        else:
            node['weight'] = 1
    else:
        g.add_node(v, weight=1)


def calculate_weights(net_path):
    g = nx.Graph()
    net_files = glob.glob("{}/place_net*.csv".format(net_path))
    t = tqdm(total=len(net_files))
    t.set_description("File %i")
    for i, f in enumerate(net_files):
        #print(f)
        t.set_description("File {}".format(i))
        if os.path.getsize(f) > 0:
            with open(f) as f_in:
                reader = csv.reader(f_in)
                is_first = f.endswith("_1.csv")
                for row in reader:
                    v1,v2 = row[0:2]
                    v1 = int(v1)
                    v2 = int(v2)
                    #if v1 == 1198811:
                    #    print("{} v1".format(f))
                    #elif v2 == 1198811:
                    #    print("{} v2".format(f))
                    update_vertex_weight(g, v2)
                    if is_first:
                        update_vertex_weight(g, v1)
                    add_edge(g, v1, v2)
        t.update()
    t.close()

    return g

def write_metis_file(g, nid_to_i, i_to_nid, outdir):
    f = "{}/place_network.graph".format(outdir)
    print("Writing metis format output to: {}".format(f))
    with open(f, 'w') as f_out:
        f_out.write("{} {} 011\n".format(len(nid_to_i), g.number_of_edges()))
        for i in range(1, len(nid_to_i) + 1):
            #row = places.loc[places['vid'] == i]
            #place_id = row['numeric_id'].iloc[0]
            place_id = i_to_nid[i]
            if g.has_node(place_id):
                # weight of node
                weight = 1
                if 'weight' in g.node[place_id]:
                    weight = g.node[place_id]['weight']
                else:
                    print("WARNING: {} Vertex Weight is Missing".format(place_id))

                f_out.write("{}".format(weight))
                for v1, v2, data in g.edges(place_id, data=True):
                    #other_id = places.loc[places['numeric_id'] == v2]['vid'].iloc[0]
                    other_id = nid_to_i[v2]
                    f_out.write(" {} {}".format(other_id, data['weight']))
                f_out.write("\n")
            else:
                f_out.write("0\n")

    f = "{}/numeric_id_idx.csv".format(outdir)
    with open(f, 'w') as f_out:
        f_out.write("numeric_id,seq_id\n")
        for k,v in nid_to_i.items():
            f_out.write("{},{}\n".format(k,v))


def main(places_path, place_net_path):
    #places_path = "/home/nick/Documents/crx_input/southside/southside_zip16_crx_places_v3.csv"
    #place_net_path = "/home/nick/Documents/results/crx/place_net_06012018/"

    g = calculate_weights(place_net_path)
    #print ([e for e in g.edges(data=True)][0:10])
    #print(g.edges(1276805, data=True))
    nid_to_i, i_to_nid = create_place_id_maps(places_path)
    outdir = os.path.abspath(os.path.dirname(places_path))
    write_metis_file(g, nid_to_i, i_to_nid, outdir)


if __name__ == '__main__':
    if len(sys.argv) != 3:
        print("Usage: pn2metis places_path place_net_path")
    else:
        main(sys.argv[1], sys.argv[2])
