import argparse
import community
import networkx as nx
import sys
import json
import os.path

from networkx.readwrite import json_graph


def save_graph(G, cluster_list, filepath):
    d = json_graph.node_link_data(
        G,
        attrs={
            'source': 'sourceIdx',
            'target': 'targetIdx',
            'key': 'key',
            'id': 'idx'
        })

    del d['directed']
    del d['multigraph']
    del d['graph']

    d['clusters'] = cluster_list

    print('Saving {}'.format(filepath))
    with open(filepath, 'w') as f:
        json.dump(d, f)


def hierarchical_clustering(G, resolution=1):
    for n in G.nodes_iter():
        G.node[n]['ancIdxs'] = []

    dendo = community.generate_dendrogram(G, resolution=resolution)
    num_levels = len(dendo)
    clusters_per_level = []
    for level in range(num_levels):
        partition = community.partition_at_level(dendo, level)
        clusters = list(set(partition.values()))
        clusters_per_level.append(clusters)
        print('clusters at level', level, 'are', clusters)
        for n, c in partition.items():
            G.node[n]['ancIdxs'].append(c)

    num_nodes = nx.number_of_nodes(G)

    def get_cluster_idx(level, idx):
        offset = num_nodes
        for i in range(level):
            offset += len(clusters_per_level[i])
        return offset + idx

    cluster_list = []
    for n in G.nodes_iter():
        node = G.node[n]
        node_clusters = node['ancIdxs']
        for level in range(len(node['ancIdxs'])):
            node_clusters[level] = get_cluster_idx(level, node_clusters[level])

        cluster_list.append({
            'idx': n,
            'nodeIdx': n,
            'parentIdx': node_clusters[0],
            'height': 0
        })

    for level, clusters in enumerate(clusters_per_level):
        for c in clusters:
            cluster_list.append({
                'idx': get_cluster_idx(level, c),
                'height': level + 1
            })

    for n in G.nodes_iter():
        node = G.node[n]
        node_clusters = node['ancIdxs']
        for level in range(len(node_clusters) - 1):
            cluster_list[node_clusters[level]]['parentIdx'] = node_clusters[
                level + 1]

    # Root
    root_cluster_idx = len(cluster_list)
    cluster_list.append({
        'idx': root_cluster_idx,
        'height': len(clusters_per_level) + 1
    })

    for c in clusters_per_level[-1]:
        cluster_list[get_cluster_idx(num_levels - 1, c)][
            'parentIdx'] = root_cluster_idx

    return cluster_list


def parse_json_d3(filepath):
    print('Reading {}'.format(filepath))

    G = nx.Graph()

    with open(filepath) as f:
        d = json.load(f)

    n_idx = {node['id']: idx for idx, node in enumerate(d['nodes'])}
    G.add_nodes_from([(idx, {'label': label}) for label, idx in n_idx.items()])
    G.add_edges_from([(n_idx[e['source']], n_idx[e['target']])
                      for e in d['links']])
    return G


def make_argparser():
    p = argparse.ArgumentParser()
    p.add_argument('filepath', type=str, help='input filepath')
    p.add_argument('-r', '--resolution', type=float,
                   help='resolution parameter for hierarchical clustering', default=1.0)
    return p


def main():
    argparser = make_argparser()

    if len(sys.argv) == 1:
        argparser.print_help()
        return

    args = argparser.parse_args()

    if '.json' in args.filepath:
        G = parse_json_d3(args.filepath)
    else:
        argparser.print_help()
        return

    cluster_list = hierarchical_clustering(G, resolution=args.resolution)

    basename = os.path.basename(args.filepath)
    filename = os.path.splitext(basename)[0]
    save_graph(G, cluster_list,
               '../Saved/Data/Graph/{}'.format(filename + '.igv.json'))


if __name__ == '__main__':
    main()
