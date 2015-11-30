/*
 * ***** BEGIN GPL LICENSE BLOCK *****
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * The Original Code is Copyright (C) Blender Foundation.
 * All rights reserved.
 *
 * The Original Code is: all of this file.
 *
 * Contributor(s): Lukas Toenne
 *
 * ***** END GPL LICENSE BLOCK *****
 */

/** \file bvm_codegen.cc
 *  \ingroup bvm
 */

#include <cstdio>
#include <set>

#include "bvm_codegen.h"
#include "bvm_eval.h"
#include "bvm_function.h"
#include "bvm_nodegraph.h"

namespace bvm {

BVMCompiler::BVMCompiler() :
    fn(NULL)
{
	stack_users.resize(BVM_STACK_SIZE, 0);
}

BVMCompiler::~BVMCompiler()
{
}

StackIndex BVMCompiler::find_stack_index(int size) const
{
	int unused = 0;
	
	for (int i = 0; i < BVM_STACK_SIZE; ++i) {
		if (stack_users[i] == 0) {
			++unused;
			if (unused == size)
				return i + 1 - size;
		}
		else
			unused = 0;
	}
	
	// TODO better reporting ...
	printf("ERROR: out of stack space");
	
	return BVM_STACK_INVALID;
}

StackIndex BVMCompiler::assign_stack_index(const TypeDesc &typedesc)
{
	int size = typedesc.stack_size();
	
	StackIndex stack_offset = find_stack_index(size);
	for (int i = 0; i < size; ++i) {
		// TODO keep track of value users
		stack_users[stack_offset + i] += 1;
	}
	
	return stack_offset;
}

void BVMCompiler::push_opcode(OpCode op)
{
	fn->add_instruction(op);
}

void BVMCompiler::push_stack_index(StackIndex arg)
{
	if (arg != BVM_STACK_INVALID)
		fn->add_instruction(arg);
}

void BVMCompiler::push_jump_address(int address)
{
	fn->add_instruction(int_to_instruction(address));
}

void BVMCompiler::push_float(float f)
{
	fn->add_instruction(float_to_instruction(f));
}

void BVMCompiler::push_float3(float3 f)
{
	fn->add_instruction(float_to_instruction(f.x));
	fn->add_instruction(float_to_instruction(f.y));
	fn->add_instruction(float_to_instruction(f.z));
}

void BVMCompiler::push_float4(float4 f)
{
	fn->add_instruction(float_to_instruction(f.x));
	fn->add_instruction(float_to_instruction(f.y));
	fn->add_instruction(float_to_instruction(f.z));
	fn->add_instruction(float_to_instruction(f.w));
}

void BVMCompiler::push_int(int i)
{
	fn->add_instruction(int_to_instruction(i));
}

void BVMCompiler::push_matrix44(matrix44 m)
{
	fn->add_instruction(float_to_instruction(m.data[0][0]));
	fn->add_instruction(float_to_instruction(m.data[0][1]));
	fn->add_instruction(float_to_instruction(m.data[0][2]));
	fn->add_instruction(float_to_instruction(m.data[0][3]));
	fn->add_instruction(float_to_instruction(m.data[1][0]));
	fn->add_instruction(float_to_instruction(m.data[1][1]));
	fn->add_instruction(float_to_instruction(m.data[1][2]));
	fn->add_instruction(float_to_instruction(m.data[1][3]));
	fn->add_instruction(float_to_instruction(m.data[2][0]));
	fn->add_instruction(float_to_instruction(m.data[2][1]));
	fn->add_instruction(float_to_instruction(m.data[2][2]));
	fn->add_instruction(float_to_instruction(m.data[2][3]));
	fn->add_instruction(float_to_instruction(m.data[3][0]));
	fn->add_instruction(float_to_instruction(m.data[3][1]));
	fn->add_instruction(float_to_instruction(m.data[3][2]));
	fn->add_instruction(float_to_instruction(m.data[3][3]));
}

void BVMCompiler::push_pointer(PointerRNA p)
{
	fn->add_instruction(pointer_to_instruction_hi(p.id.data));
	fn->add_instruction(pointer_to_instruction_lo(p.id.data));
	fn->add_instruction(pointer_to_instruction_hi(p.type));
	fn->add_instruction(pointer_to_instruction_lo(p.type));
	fn->add_instruction(pointer_to_instruction_hi(p.data));
	fn->add_instruction(pointer_to_instruction_lo(p.data));
}

StackIndex BVMCompiler::codegen_value(const Value *value)
{
	StackIndex offset = assign_stack_index(value->typedesc());
	
	switch (value->typedesc().base_type) {
		case BVM_FLOAT: {
			float f = 0.0f;
			value->get(&f);
			
			push_opcode(OP_VALUE_FLOAT);
			push_float(f);
			push_stack_index(offset);
			break;
		}
		case BVM_FLOAT3: {
			float3 f = float3(0.0f, 0.0f, 0.0f);
			value->get(&f);
			
			push_opcode(OP_VALUE_FLOAT3);
			push_float3(f);
			push_stack_index(offset);
			break;
		}
		case BVM_FLOAT4: {
			float4 f = float4(0.0f, 0.0f, 0.0f, 0.0f);
			value->get(&f);
			
			push_opcode(OP_VALUE_FLOAT4);
			push_float4(f);
			push_stack_index(offset);
			break;
		}
		case BVM_INT: {
			int i = 0;
			value->get(&i);
			
			push_opcode(OP_VALUE_INT);
			push_int(i);
			push_stack_index(offset);
			break;
		}
		case BVM_MATRIX44: {
			matrix44 m = matrix44::identity();
			value->get(&m);
			
			push_opcode(OP_VALUE_MATRIX44);
			push_matrix44(m);
			push_stack_index(offset);
			break;
		}
		case BVM_POINTER: {
			PointerRNA p = PointerRNA_NULL;
			value->get(&p);
			
			push_opcode(OP_VALUE_POINTER);
			push_pointer(p);
			push_stack_index(offset);
			break;
		}
		
		case BVM_MESH:
			push_opcode(OP_VALUE_MESH);
			push_stack_index(offset);
			break;
	}
	
	return offset;
}

void BVMCompiler::codegen_constant(const Value *value)
{
	switch (value->typedesc().base_type) {
		case BVM_FLOAT: {
			float f = 0.0f;
			value->get(&f);
			
			push_float(f);
			break;
		}
		case BVM_FLOAT3: {
			float3 f = float3(0.0f, 0.0f, 0.0f);
			value->get(&f);
			
			push_float3(f);
			break;
		}
		case BVM_FLOAT4: {
			float4 f = float4(0.0f, 0.0f, 0.0f, 0.0f);
			value->get(&f);
			
			push_float4(f);
			break;
		}
		case BVM_INT: {
			int i = 0;
			value->get(&i);
			
			push_int(i);
			break;
		}
		case BVM_MATRIX44: {
			matrix44 m = matrix44::identity();
			value->get(&m);
			
			push_matrix44(m);
			break;
		}
		case BVM_POINTER: {
			PointerRNA p = PointerRNA_NULL;
			value->get(&p);
			
			push_pointer(p);
			break;
		}
		
		case BVM_MESH:
			break;
	}
}

static OpCode ptr_init_opcode(const TypeDesc &typedesc)
{
	switch (typedesc.base_type) {
		case BVM_FLOAT:
		case BVM_FLOAT3:
		case BVM_FLOAT4:
		case BVM_INT:
		case BVM_MATRIX44:
		case BVM_POINTER:
			return OP_NOOP;
		
		case BVM_MESH:
			return OP_INIT_MESH_PTR;
	}
	return OP_NOOP;
}

static OpCode ptr_release_opcode(const TypeDesc &typedesc)
{
	switch (typedesc.base_type) {
		case BVM_FLOAT:
		case BVM_FLOAT3:
		case BVM_FLOAT4:
		case BVM_INT:
		case BVM_MATRIX44:
		case BVM_POINTER:
			return OP_NOOP;
		
		case BVM_MESH:
			return OP_RELEASE_MESH_PTR;
	}
	return OP_NOOP;
}

int BVMCompiler::codegen_subgraph(const NodeList &nodes,
                                  const SocketUserMap &socket_users,
                                  SubgraphOutputList &outputs)
{
	typedef std::map<ConstSocketPair, StackIndex> SocketIndexMap;
	
	int entry_point = fn->get_instruction_count();
	
	SocketIndexMap output_index;
	for (NodeList::const_iterator it = nodes.begin(); it != nodes.end(); ++it) {
		const NodeInstance &node = **it;
		
		OpCode op = get_opcode_from_node_type(node.type->name());
		if (op == OP_NOOP)
			continue;
		
		/* prepare input stack entries */
		SocketIndexMap input_index;
		for (int i = 0; i < node.num_inputs(); ++i) {
			const NodeSocket *input = node.type->find_input(i);
			ConstSocketPair key(&node, input->name);
			assert(input_index.find(key) == input_index.end());
			
			if (node.is_input_constant(i) || node.is_input_function(i)) {
				/* stored directly in the instructions list after creating values */
			}
			else if (node.has_input_link(i)) {
				ConstSocketPair link_key(node.find_input_link_node(i),
				                         node.find_input_link_socket(i)->name);
				assert(output_index.find(link_key) != output_index.end());
				input_index[key] = output_index[link_key];
			}
			else if (node.has_input_value(i)) {
				Value *value = node.find_input_value(i);
				input_index[key] = codegen_value(value);
			}
			else {
				input_index[key] = codegen_value(input->default_value);
			}
		}
		
		/* initialize output data stack entries */
		for (int i = 0; i < node.num_outputs(); ++i) {
			const NodeSocket *output = node.type->find_output(i);
			ConstSocketPair key(&node, output->name);
			
			output_index[key] = assign_stack_index(output->typedesc);
			
			/* if necessary, add a user count initializer */
			OpCode init_op = ptr_init_opcode(output->typedesc);
			if (init_op != OP_NOOP) {
				assert(socket_users.find(key) != socket_users.end());
				int users = socket_users.find(key)->second;
				if (users > 0) {
					push_opcode(init_op);
					push_stack_index(output_index[key]);
					push_int(users);
				}
			}
		}
		
		/* write main opcode */
		push_opcode(op);
		/* write input stack offsets and constants */
		for (int i = 0; i < node.num_inputs(); ++i) {
			const NodeSocket *input = node.type->find_input(i);
			ConstSocketPair key(&node, input->name);
			
			if (node.is_input_constant(i)) {
				Value *value = node.find_input_value(i);
				codegen_constant(value);
			}
			else if (node.is_input_function(i)) {
				assert(func_entry_map.find(key) != func_entry_map.end());
				FunctionInfo &func = func_entry_map[key];
				push_jump_address(func.entry_point);
				push_stack_index(func.return_index);
			}
			else {
				assert(input_index.find(key) != input_index.end());
				push_stack_index(input_index[key]);
			}
		}
		/* write output stack offsets */
		for (int i = 0; i < node.num_outputs(); ++i) {
			const NodeSocket *output = node.type->find_output(i);
			ConstSocketPair key(&node, output->name);
			assert(output_index.find(key) != output_index.end());
			
			push_stack_index(output_index[key]);
		}
		
		/* release input data stack entries */
		for (int i = 0; i < node.num_inputs(); ++i) {
			const NodeSocket *input = node.type->find_input(i);
			
			if (node.is_input_constant(i) || node.is_input_function(i)) {
				/* pass */
			}
			else if (node.has_input_link(i)) {
				ConstSocketPair link_key(node.find_input_link_node(i),
				                         node.find_input_link_socket(i)->name);
				assert(output_index.find(link_key) != output_index.end());
				
				OpCode release_op = ptr_release_opcode(input->typedesc);
				
				if (release_op != OP_NOOP) {
					push_opcode(release_op);
					push_stack_index(output_index[link_key]);
				}
			}
		}
	}
	
	push_opcode(OP_END);
	
	for (SubgraphOutputList::iterator it = outputs.begin(); it != outputs.end(); ++it) {
		SubgraphOutput &output = *it;
		
		if (output.key.node) {
			assert(output_index.find(output.key) != output_index.end());
			output.stack_index = output_index[output.key];
		}
		else {
			output.stack_index = codegen_value(output.value);
		}
	}
	
	return entry_point;
}

void BVMCompiler::graph_node_append(const NodeInstance *node,
                                    NodeList &sorted_nodes,
                                    NodeSet &visited)
{
	if (visited.find(node) != visited.end())
		return;
	visited.insert(node);
	
	for (size_t i = 0; i < node->num_inputs(); ++i) {
		const NodeSocket *socket = node->type->find_input(i);
		if (socket->value_type == VALUE_FUNCTION) {
			func_entry_map[node->input(i)] = FunctionInfo();
		}
		else {
			const NodeInstance *link_node = node->find_input_link_node(i);
			if (link_node) {
				graph_node_append(link_node, sorted_nodes, visited);
			}
		}
	}
	
	sorted_nodes.push_back(node);
}

void BVMCompiler::sort_graph_nodes(const NodeGraph &graph,
                                   NodeList &sorted_nodes)
{
	NodeSet visited;
	
	for (NodeGraph::NodeInstanceMap::const_iterator it = graph.nodes.begin(); it != graph.nodes.end(); ++it) {
		graph_node_append(it->second, sorted_nodes, visited);
	}
}

static void expression_node_append(const NodeInstance *node,
                                   NodeList &sorted_nodes,
                                   NodeSet &visited)
{
	if (node->type->is_kernel_node())
		return;
	
	if (visited.find(node) != visited.end())
		return;
	visited.insert(node);
	
	for (size_t i = 0; i < node->num_inputs(); ++i) {
		const NodeInstance *link_node = node->find_input_link_node(i);
		if (link_node) {
			expression_node_append(link_node, sorted_nodes, visited);
		}
	}
	
	sorted_nodes.push_back(node);
}

static void sort_expression_nodes(const ConstSocketPair &key,
                                  NodeList &sorted_nodes)
{
	NodeSet visited;
	
	if (key.node->has_input_link(key.socket)) {
		const NodeInstance *link_node = key.node->find_input_link_node(key.socket);
		expression_node_append(link_node, sorted_nodes, visited);
	}
}

static void count_output_users(const NodeGraph &graph,
                               SocketUserMap &users)
{
	users.clear();
	for (NodeGraph::NodeInstanceMap::const_iterator it = graph.nodes.begin(); it != graph.nodes.end(); ++it) {
		const NodeInstance *node = it->second;
		for (int i = 0; i < node->num_outputs(); ++i) {
			ConstSocketPair key(node, node->type->find_output(i)->name);
			users[key] = 0;
		}
	}
	
	for (NodeGraph::NodeInstanceMap::const_iterator it = graph.nodes.begin(); it != graph.nodes.end(); ++it) {
		const NodeInstance *node = it->second;
		
		/* note: pass nodes are normally removed, but can exist for debugging purposes */
		if (node->type->is_pass_node())
			continue;
		
		for (int i = 0; i < node->num_inputs(); ++i) {
			if (node->has_input_link(i)) {
				ConstSocketPair key(node->find_input_link_node(i),
				                    node->find_input_link_socket(i)->name);
				users[key] += 1;
			}
		}
	}
	/* inputs are defined externally, they should be retained during evaluation */
	for (NodeGraph::InputList::const_iterator it = graph.inputs.begin(); it != graph.inputs.end(); ++it) {
		const NodeGraph::Input &input = *it;
		users[input.key] += 1;
	}
	/* outputs are passed on to the caller, which is responsible for freeing them */
	for (NodeGraph::OutputList::const_iterator it = graph.outputs.begin(); it != graph.outputs.end(); ++it) {
		const NodeGraph::Output &output = *it;
		users[output.key] += 1;
	}
}

Function *BVMCompiler::codegen_function(const NodeGraph &graph)
{
	SocketUserMap output_users;
	count_output_users(graph, output_users);
	
	NodeList main_nodes;
	sort_graph_nodes(graph, main_nodes);
	
	fn = new Function();
	
	/* first generate separate kernel functions */
	for (FunctionEntryMap::iterator it = func_entry_map.begin(); it != func_entry_map.end(); ++it) {
		const ConstSocketPair &key = it->first;
		FunctionInfo &func = it->second;
		
		NodeList expr_nodes;
		sort_expression_nodes(key, expr_nodes);
		
		/* TODO loading values from top-level nodes
		 * will not work at this point (input stack indices are unknown).
		 * for that the stack index instructions will have to be
		 * updated _after_ the main function is generated.
		 */
		
		SubgraphOutputList outputs;
		ConstSocketPair link_key = key.node->link(key.socket);
		Value *value = key.node->type->find_input(key.socket)->default_value;
		outputs.push_back(SubgraphOutput(link_key, value));
		
		func.entry_point = codegen_subgraph(expr_nodes, output_users, outputs);
		func.return_index = outputs[0].stack_index;
	}
	
	/* now generate the main function */
	{
		SubgraphOutputList outputs;
		for (size_t i = 0; i < graph.outputs.size(); ++i) {
			const NodeGraph::Output &output = graph.outputs[i];
			Value *value = output.key.node->type->find_output(output.key.socket)->default_value;
			outputs.push_back(SubgraphOutput(output.key, value));
		}
		int entry_point = codegen_subgraph(main_nodes, output_users, outputs);
		fn->set_entry_point(entry_point);
		
		/* store final stack indices for outputs, so we can return results to the caller */
		for (size_t i = 0; i < graph.outputs.size(); ++i) {
			const NodeGraph::Output &output = graph.outputs[i];
			const NodeSocket *socket = output.key.node->type->find_output(output.key.socket);
			
			fn->add_return_value(socket->typedesc, output.name, outputs[i].stack_index);
		}
	}
	
	Function *result = fn;
	fn = NULL;
	return result;
}

} /* namespace bvm */
