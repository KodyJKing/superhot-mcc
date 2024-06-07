'''
    Parse a Cheat Engine structure xml file and convert it to a C structure.

    Sample file:

    <Structures>
        <Structure Name="Entity" AutoFill="0" AutoCreate="1" DefaultHex="0" AutoDestroy="0" DoNotSaveLocal="0" RLECompression="1" AutoCreateStructsize="4096">
            <Elements>
                <Element Offset="0" Vartype="2 Bytes" Bytesize="2" OffsetHex="00000000" Description="tagID" DisplayMethod="hexadecimal"/>
                <Element Offset="2" Vartype="2 Bytes" Bytesize="2" OffsetHex="00000002" DisplayMethod="hexadecimal"/>
                <Element Offset="4" Vartype="4 Bytes" Bytesize="4" RLECount="3" OffsetHex="00000004" DisplayMethod="unsigned integer"/>
                <Element Offset="16" Vartype="4 Bytes" Bytesize="4" OffsetHex="00000010" DisplayMethod="hexadecimal"/>
                <Element Offset="20" Vartype="4 Bytes" Bytesize="4" OffsetHex="00000014" Description="ageMilis" DisplayMethod="unsigned integer"/>
                <Element Offset="24" Vartype="Float" Bytesize="4" OffsetHex="00000018" Description="pos.x" DisplayMethod="unsigned integer"/>
                <Element Offset="28" Vartype="Float" Bytesize="4" OffsetHex="0000001C" Description="pos.y" DisplayMethod="unsigned integer"/>
                <Element Offset="32" Vartype="Float" Bytesize="4" OffsetHex="00000020" Description="pos.z" DisplayMethod="unsigned integer"/>
                <Element Offset="36" Vartype="Float" Bytesize="4" OffsetHex="00000024" Description="vel.x" DisplayMethod="unsigned integer"/>
                <Element Offset="40" Vartype="Float" Bytesize="4" OffsetHex="00000028" Description="vel.y" DisplayMethod="unsigned integer"/>
                <Element Offset="44" Vartype="Float" Bytesize="4" OffsetHex="0000002C" Description="vel.z" DisplayMethod="unsigned integer"/>
                <Element Offset="48" Vartype="Float" Bytesize="4" OffsetHex="00000030" Description="fwd.x" DisplayMethod="unsigned integer"/>
                <Element Offset="52" Vartype="Float" Bytesize="4" OffsetHex="00000034" Description="fwd.y" DisplayMethod="unsigned integer"/>
                <Element Offset="56" Vartype="Float" Bytesize="4" OffsetHex="00000038" Description="fwd.z" DisplayMethod="unsigned integer"/>
                <Element Offset="60" Vartype="Float" Bytesize="4" OffsetHex="0000003C" Description="up.x" DisplayMethod="unsigned integer"/>
                <Element Offset="64" Vartype="Float" Bytesize="4" OffsetHex="00000040" Description="up.y" DisplayMethod="unsigned integer"/>
                <Element Offset="68" Vartype="Float" Bytesize="4" OffsetHex="00000044" Description="up.z" DisplayMethod="unsigned integer"/>
                <Element Offset="72" Vartype="Float" Bytesize="4" OffsetHex="00000048" Description="angularVel.x" DisplayMethod="unsigned integer"/>
                <Element Offset="76" Vartype="Float" Bytesize="4" OffsetHex="0000004C" Description="angularVel.y" DisplayMethod="unsigned integer"/>
                <Element Offset="80" Vartype="Float" Bytesize="4" OffsetHex="00000050" Description="angularVel.z" DisplayMethod="unsigned integer"/>
            </Elements>
        </Structure>
    </Structures>
'''

import xml.etree.ElementTree as ET
import re
import os

def parse_structure(structure):
    elements = []
    for element in structure.findall('Elements/Element'):
        offset = int(element.get('Offset'))
        vartype = element.get('Vartype')
        bytesize = int(element.get('Bytesize'))
        description = element.get('Description', '')
        displaymethod = element.get('DisplayMethod', '')
        if not description == '':
            elements.append((offset, vartype, bytesize, description, displaymethod))
    return elements

def format_identifier(description):
    # Split on non-alphanumeric characters. Output camelCase.
    words = re.split(r'\W+', description)
    first = words[0][0].lower() + words[0][1:]
    return first + ''.join([word.capitalize() for word in words[1:]])

def get_c_type(vartype, displaymethod, bytesize):
    isSigned = 'unsigned' not in displaymethod
    if vartype == 'Byte':
        return 'uint8_t' if isSigned else 'int8_t'
    if vartype == '2 Bytes':
        return 'uint16_t' if isSigned else 'int16_t'
    if vartype == '4 Bytes':
        return 'uint32_t' if isSigned else 'int32_t'
    if vartype == '8 Bytes':
        return 'uint64_t' if isSigned else 'int64_t'
    if vartype == 'Float':
        return 'float'
    if vartype == 'Double':
        return 'double'
    return vartype

def get_c_fields(structure):
    elements = parse_structure(structure)
    fields = []

    running_offset = 0
    for element in elements:
        offset, vartype, bytesize, description, displaymethod = element

        # Add padding if necessary.
        if offset > running_offset:
            padding = offset - running_offset
            line = '    uint8_t pad_%04X[%d];' % (offset - padding, padding)
            fields.append((line, running_offset))
            running_offset = offset
        
        identifier = format_identifier(description)
        ctype = get_c_type(vartype, displaymethod, bytesize)
        line = '    %s %s;' % (ctype, identifier)
        fields.append((line, offset))
        
        running_offset += bytesize
        
    return fields

def struct_to_c(structure):
    c_fields = get_c_fields(structure)

    max_line = max([len(line) for line, _ in c_fields])

    c_struct = '#pragma pack(push, 1)\nstruct %s {\n' % structure.get('Name')

    for line, offset in c_fields:
        c_struct += line.ljust(max_line) + ' // %04X\n' % offset
    
    c_struct += '};\n#pragma pack(pop)\n'

    return c_struct

def convert_file(filepath):
    print("Converting file: %s" % filepath)
    tree = ET.parse(filepath)
    root = tree.getroot()
    output = '#include <stdint.h>\n\n'
    for structure in root.findall('Structure'):
        cstruct = struct_to_c(structure)
        output += cstruct + '\n'
        print(cstruct)
    
    filename = os.path.basename(filepath)
    filename = os.path.splitext(filename)[0]
    filename = filename + '.c'
    with open(filename, 'w') as f:
        f.write(output)

    return output

if __name__ == '__main__':
    import sys

    if len(sys.argv) == 2:
        convert_file(sys.argv[1])
    else:
        for filename in os.listdir('.'):
            if filename.endswith('.CSX'):
                convert_file(filename)
