#include "EDAUtils.h"
#include <algorithm>
#include <list>
#include <map>

using namespace std;

/* const static string PPO_PREFIX = ""; */

const std::string EDAUtils::PPO_PREFIX = "PPO:";
const std::string EDAUtils::PPI_PREFIX = "PPI:";
void EDAUtils::levelize(const Circuit &circuit)
{
    cout << "Call EDAUtils::levelize()" << endl;

    list<Node*> Queue;
    map<string, unsigned> count_map;
    
    for(size_t c_idx = 0; c_idx < circuit.topModule().cellSize(); c_idx++)
    {
        Cell cell = circuit.topModule().cell(c_idx);
        for(size_t p_idx = 0; p_idx < cell.inputSize(); p_idx++)
        {
            Node node = cell.input(p_idx);
            if(node.isWire())
            {
                Wire w = node.toWire();
                for(size_t w_idx = 0; w_idx < w.inputSize(); w_idx++) // one input
                {
                    Node nodew = w.input(w_idx);
                    if(nodew.isWire())
                    {
                        cout << "levelize Exception: Wire to Wire" << endl;
                    }
                    else if(nodew.isCell())
                    {
                        Cell nodec = nodew.toCell();
                        if(nodec.type().find("FF") && nodec.hasInput("CK"))
                        {
                            if(count_map.count(cell.name()) == 0)
                                count_map[cell.name()] = 1;
                            else
                                count_map[cell.name()] = count_map[cell.name()] + 1;
                            if(count_map[cell.name()] == cell.inputSize())
                            {
                                cell.setLevel(1);
                                Queue.push_back(new Cell(cell));
                            }
                        }
                    }
                }
            }
            else if(node.isCell())
            {
                Cell nodec = node.toCell();
                if(nodec.type().find("FF") && nodec.hasInput("CK"))
                {
                    if(count_map.count(cell.name()) == 0)
                        count_map[cell.name()] = 1;
                    else
                        count_map[cell.name()] = count_map[cell.name()] + 1;
                    if(count_map[cell.name()] == cell.inputSize())
                    {
                        cell.setLevel(1);
                        Queue.push_back(new Cell(cell));
                    }
                }
            }
        }
    }

    for(size_t in_idx = 0; in_idx < circuit.inputSize(); in_idx++)
    {  
        Port p = circuit.inputPort(in_idx);
        for(size_t pin_idx = 0; pin_idx < p.outputSize(); pin_idx++)
        {
            Node node = p.output(pin_idx);
            if(node.isCell())
            {
                Cell cell = node.toCell();
                if(count_map.count(cell.name()) == 0)
                    count_map[cell.name()] = 1;
                else
                    count_map[cell.name()] = count_map[cell.name()] + 1;

                if(count_map[cell.name()] == cell.inputSize())
                {
                    cell.setLevel(1);
                    Queue.push_back(new Cell(cell));
                }
            }
            else if(node.isGate())
            {
                Gate gate = node.toGate();
                if(count_map.count(gate.name()) == 0)
                    count_map[gate.name()] = 1;
                else
                    count_map[gate.name()] = count_map[gate.name()] + 1;
                if(count_map[gate.name()] == gate.inputSize())
                {
                    gate.setLevel(1);
                    Queue.push_back(new Gate(gate));
                }
            }
            else if(node.isPort())
                cout << "Input port directly connect to output port" << endl;
            else
                cout << "Illegal levelize Type" << endl;
        }
    }


    int l1, l2;
    while(!Queue.empty())
    {
        Node* node = Queue.front();
        Queue.pop_front();
        if(node->isCell())
            l2 = node->toCell().level();
        else if(node->isGate())
            l2 = node->toGate().level();

        for(size_t j = 0; j < node->outputSize(); j++)
        {
            Node out = node->output(j);
            if(out.isWire())
            {
                Wire w = out.toWire();
                for(size_t k = 0; k < w.outputSize(); k++)
                {
                    Node outw = w.output(k);
                    if(outw.isCell())
                    {
                        Cell outc = outw.toCell();
                        l1 = outc.level();
                        if(l1 <= l2)
                            outc.setLevel(l2+1);
                        count_map[outc.name()] = count_map[outc.name()] + 1;
                        if(count_map[outc.name()] == outc.inputSize())
                            Queue.push_back(new Cell(outc));
                    }
                    else if(outw.isGate())
                    {
                        Gate outg = outw.toGate();
                        l1 = outg.level();
                        if(l1 <= l2)
                            outg.setLevel(l2+1);
                        count_map[outg.name()] = count_map[outg.name()] + 1;
                        if(count_map[outg.name()] == outg.inputSize())
                            Queue.push_back(new Gate(outg));
                    }
                }
            }
            else if(out.isPort())
            {
                Port p = out.toPort();
                for(size_t k = 0; k < p.outputSize(); k++)
                {
                    Node outp = p.output(k);
                    if(outp.isCell())
                    {
                        Cell outc = outp.toCell();
                        l1 = outc.level();
                        if(l1 <= l2)
                            outc.setLevel(l2+1);
                    }
                    else if(outp.isGate())
                    {
                        Gate outg = outp.toGate();
                        l1 = outg.level();
                        if(l1 <= l2)
                            outg.setLevel(l2+1);
                    }
                }
            }
            else if(out.isCell())
            {
                Cell outc = out.toCell();
                l1 = outc.level();
                if(l1 <= l2)
                    outc.setLevel(l2+1);
                count_map[outc.name()] = count_map[outc.name()] + 1;
                if(count_map[outc.name()] == outc.inputSize())
                    Queue.push_back(new Cell(outc));
            }
            else if(out.isGate())
            {
                Gate outg = out.toGate();
                l1 = outg.level();
                if(l1 <= l2)
                    outg.setLevel(l2+1);
                count_map[outg.name()] = count_map[outg.name()] + 1;
                if(count_map[outg.name()] == outg.inputSize())
                    Queue.push_back(new Gate(outg));
            }
        }
    }
}

static 
bool _compare_(const Gate &gate1, const Gate &gate2)
{
    return gate1.level() < gate2.level();
}
void EDAUtils::orderByLevel(const Circuit &circuit, std::vector<Gate> &gates)
{
    levelize(circuit);
    for(size_t idx = 0; idx < circuit.topModule().gateSize(); idx++)
        gates.push_back(circuit.topModule().gate(idx));
    std::sort(gates.begin(), gates.end(), _compare_);
}

void EDAUtils::orderByLevel(const Circuit &circuit, std::vector<Cell> &cells)
{
    levelize(circuit);
    for(size_t idx = 0; idx < circuit.topModule().cellSize(); idx++)
        cells.push_back(circuit.topModule().cell(idx));
    std::sort(cells.begin(), cells.end(), _compare_);
}

void EDAUtils::removeAllDFF(Circuit &circuit)
{
    for(size_t m_idx = 0; m_idx < circuit.moduleSize(); m_idx++)
    {
        Module module = circuit.module(m_idx);
        for(size_t c_idx = 0; c_idx < module.cellSize(); c_idx++)
        {
            Cell cell = module.cell(c_idx);
            if(cell.type().find("FF") && cell.hasInput("CK"))
            {
                module.removeNode(cell);

                for(size_t pin_i = 0; pin_i < cell.inputSize(); pin_i++)
                {
                    if(cell.inputPinName(pin_i) == "CK") continue;

                    Node nodei = cell.input(pin_i); // Port or Wire
                    if(!nodei.isNull())
                    {
                        Port ppo = module.createPort(PPO_PREFIX + nodei.name(), Port::PortType::PPO);
                        nodei.connect(Node::dir2str(Node::Direct::right), ppo);
                    }
                }
                for(size_t pin_o = 0; pin_o < cell.outputSize(); pin_o++)
                {
                    Node nodeo = cell.output(pin_o); // Wire
                    if(!nodeo.isNull())
                    {
                        Port ppi = module.createPort(PPI_PREFIX + nodeo.name(), Port::PortType::PPI);
                        nodeo.connect(Node::dir2str(Node::Direct::left), ppi);
                    }
                }
            }
        }
    }
}

void EDAUtils::timeFrameExpansion(Circuit &circuit, unsigned cycles)
{
    
}
