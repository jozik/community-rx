import pandas as pd
import os, sys


def write_to_csv(df, file_path, name, p_count):
    idx = file_path.rfind(".")
    f = "{}_p{}{}".format(file_path[0:idx], p_count, file_path[idx:])
    print("Writing {} to {}".format(name, f))
    df.to_csv(f, index=False)


def main(persons_path, places_path, process_count):
    places = pd.read_csv(places_path)
    persons = pd.read_csv(persons_path)

    data_dir = os.path.abspath(os.path.dirname(places_path))
    conc = pd.read_csv("{}/numeric_id_idx.csv".format(data_dir))
    ranks = pd.read_csv("{}/place_network.graph.part.{}".format(data_dir,
        process_count), header=None)
    ranks['seq_id'] = ranks.index + 1
    ranks.columns = ['rank', 'seq_id']
    ranks = ranks.join(conc, rsuffix='r')

    places['rank'] = places['numeric_id'].map(ranks.set_index('numeric_id')['rank'])
    persons['rank'] = persons['hh_id'].map(ranks.set_index('numeric_id')['rank'])

    write_to_csv(places, places_path, 'places', process_count)
    write_to_csv(persons, persons_path, 'persons', process_count)


if __name__ == '__main__':
    if len(sys.argv) != 4:
        print("Usage: assign_ranks_from_metis_output.py 'persons_file' 'places_file' 'process count'")
    else:
        main(sys.argv[1], sys.argv[2], int(sys.argv[3]))
