from p4utils.mininetlib.network_API import NetworkAPI
import networkx as nx 
import matplotlib.pyplot as plt 
import os

# VARIABLES 
hosts_per_switch = 3
n_switches = 1
####################


def save_graph(G): 
    #Saving the picture of the topology in a folder 
    pos = nx.spring_layout(G)
    
    nx.draw(G, pos, labels={node:node for node in G.nodes()})
    plt.title('Network Topology')
    output_file = 'topology.png'

    path = os.getcwd() + '/pictures' #getting the current directory 
    if not os.path.exists(path):
        os.makedirs(path)
        print(f"folder {path} created.")
    else: 
        print(f'folder {path} already existed')

    plt.savefig(path + '/'+ output_file, format= "PNG")
    print("GRAPH SAVED")

def main(): 
    net = NetworkAPI()

    # Network general options
    net.setLogLevel('info')
    net.enableCli()

    # Network definition
    switches = [f"s{i}" for i in range(n_switches)]

    #Creating concurrently the graph
    G = nx.Graph()

    for switch in switches: 
        print(switch)
        #net.addP4Switch(switch, cli_input='s1-commands.txt')
        net.addP4Switch(switch)
        #net.addP4Switch(switch, cli_input='cliconfig.txt')
        G.add_node(switch, type='switch')
        for host in range(hosts_per_switch):
            name = f"{switch}h{host}"
            net.addHost(name)
            net.addLink(switch, name)
            G.add_node(switch, type='host')
            G.add_edge(switch, name)
    
    #public_attrs = [attr for attr in dir(NetworkAPI) if not attr.startswith('_')]
    #print(public_attrs)
   
    
    # Connect all switches to each other without duplicates
    for i in range(len(switches)):
        for j in range(i + 1, len(switches)):
            net.addLink(switches[i], switches[j])
            G.add_edge(switches[i], switches[j])
    


    net.setP4SourceAll('p4src/connecting.p4')


    save_graph(G)

    # Assignment strategy
    net.l2()

    # Nodes general options
    net.enablePcapDumpAll()
    net.enableLogAll()

    # Start network
    net.startNetwork()


if __name__ == "__main__":
    main()
