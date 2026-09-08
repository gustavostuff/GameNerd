from collections import defaultdict
from skidl import Pin, Part, Alias, SchLib, SKIDL, TEMPLATE

from skidl.pin import pin_types

SKIDL_lib_version = '0.0.1'

generate = SchLib(tool=SKIDL).add_parts(*[
        Part(**{ 'name':'CART_EDGE_16', 'dest':TEMPLATE, 'tool':SKIDL, 'aliases':Alias({'CART_EDGE_16'}), 'ref_prefix':'U', 'fplist':None, 'footprint':'Retr01_Lib:Cart_Edge_2x8_P2.54mm', 'keywords':None, 'description':'', 'datasheet':None, 'pins':[
            Pin(num='1',name='1',func=pin_types.UNSPEC),
            Pin(num='2',name='2',func=pin_types.UNSPEC),
            Pin(num='3',name='3',func=pin_types.UNSPEC),
            Pin(num='4',name='4',func=pin_types.UNSPEC),
            Pin(num='5',name='5',func=pin_types.UNSPEC),
            Pin(num='6',name='6',func=pin_types.UNSPEC),
            Pin(num='7',name='7',func=pin_types.UNSPEC),
            Pin(num='8',name='8',func=pin_types.UNSPEC),
            Pin(num='9',name='9',func=pin_types.UNSPEC),
            Pin(num='10',name='10',func=pin_types.UNSPEC),
            Pin(num='11',name='11',func=pin_types.UNSPEC),
            Pin(num='12',name='12',func=pin_types.UNSPEC),
            Pin(num='13',name='13',func=pin_types.UNSPEC),
            Pin(num='14',name='14',func=pin_types.UNSPEC),
            Pin(num='15',name='15',func=pin_types.UNSPEC),
            Pin(num='16',name='16',func=pin_types.UNSPEC)] }),
        Part(**{ 'name':'SST25VF010A', 'dest':TEMPLATE, 'tool':SKIDL, 'aliases':Alias({'SST25VF010A'}), 'ref_prefix':'U', 'fplist':None, 'footprint':'Package_SO:SOIC-8_3.9x4.9mm_P1.27mm', 'keywords':None, 'description':'', 'datasheet':None, 'pins':[
            Pin(num='1',name='1',func=pin_types.UNSPEC),
            Pin(num='2',name='2',func=pin_types.UNSPEC),
            Pin(num='3',name='3',func=pin_types.UNSPEC),
            Pin(num='4',name='4',func=pin_types.UNSPEC),
            Pin(num='5',name='5',func=pin_types.UNSPEC),
            Pin(num='6',name='6',func=pin_types.UNSPEC),
            Pin(num='7',name='7',func=pin_types.UNSPEC),
            Pin(num='8',name='8',func=pin_types.UNSPEC)] }),
        Part(**{ 'name':'24C64', 'dest':TEMPLATE, 'tool':SKIDL, 'aliases':Alias({'24C64'}), 'ref_prefix':'U', 'fplist':None, 'footprint':'Package_SO:SOIC-8_3.9x4.9mm_P1.27mm', 'keywords':None, 'description':'', 'datasheet':None, 'pins':[
            Pin(num='1',name='1',func=pin_types.UNSPEC),
            Pin(num='2',name='2',func=pin_types.UNSPEC),
            Pin(num='3',name='3',func=pin_types.UNSPEC),
            Pin(num='4',name='4',func=pin_types.UNSPEC),
            Pin(num='5',name='5',func=pin_types.UNSPEC),
            Pin(num='6',name='6',func=pin_types.UNSPEC),
            Pin(num='7',name='7',func=pin_types.UNSPEC),
            Pin(num='8',name='8',func=pin_types.UNSPEC)] }),
        Part(**{ 'name':'C_100N', 'dest':TEMPLATE, 'tool':SKIDL, 'aliases':Alias({'C_100N'}), 'ref_prefix':'U', 'fplist':None, 'footprint':'Capacitor_THT:C_Disc_D5.0mm_W2.5mm_P5.00mm', 'keywords':None, 'description':'', 'datasheet':None, 'pins':[
            Pin(num='1',name='1',func=pin_types.UNSPEC),
            Pin(num='2',name='2',func=pin_types.UNSPEC)] }),
        Part(**{ 'name':'R_0', 'dest':TEMPLATE, 'tool':SKIDL, 'aliases':Alias({'R_0'}), 'ref_prefix':'U', 'fplist':None, 'footprint':'Resistor_THT:R_Axial_DIN0207_L6.3mm_D2.5mm_P2.54mm_Vertical', 'keywords':None, 'description':'', 'datasheet':None, 'pins':[
            Pin(num='1',name='1',func=pin_types.UNSPEC),
            Pin(num='2',name='2',func=pin_types.UNSPEC)] })])