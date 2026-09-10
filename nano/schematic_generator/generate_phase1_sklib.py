from collections import defaultdict
from skidl import Pin, Part, Alias, SchLib, SKIDL, TEMPLATE

from skidl.pin import pin_types

SKIDL_lib_version = '0.0.1'

generate_phase1 = SchLib(tool=SKIDL).add_parts(*[
        Part(**{ 'name':'ATmega1284P', 'dest':TEMPLATE, 'tool':SKIDL, 'aliases':Alias({'ATmega1284P'}), 'ref_prefix':'U', 'fplist':None, 'footprint':'Package_DIP:DIP-40_W15.24mm', 'keywords':None, 'description':'', 'datasheet':None, 'pins':[
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
            Pin(num='16',name='16',func=pin_types.UNSPEC),
            Pin(num='17',name='17',func=pin_types.UNSPEC),
            Pin(num='18',name='18',func=pin_types.UNSPEC),
            Pin(num='19',name='19',func=pin_types.UNSPEC),
            Pin(num='20',name='20',func=pin_types.UNSPEC),
            Pin(num='21',name='21',func=pin_types.UNSPEC),
            Pin(num='22',name='22',func=pin_types.UNSPEC),
            Pin(num='23',name='23',func=pin_types.UNSPEC),
            Pin(num='24',name='24',func=pin_types.UNSPEC),
            Pin(num='25',name='25',func=pin_types.UNSPEC),
            Pin(num='26',name='26',func=pin_types.UNSPEC),
            Pin(num='27',name='27',func=pin_types.UNSPEC),
            Pin(num='28',name='28',func=pin_types.UNSPEC),
            Pin(num='29',name='29',func=pin_types.UNSPEC),
            Pin(num='30',name='30',func=pin_types.UNSPEC),
            Pin(num='31',name='31',func=pin_types.UNSPEC),
            Pin(num='32',name='32',func=pin_types.UNSPEC),
            Pin(num='33',name='33',func=pin_types.UNSPEC),
            Pin(num='34',name='34',func=pin_types.UNSPEC),
            Pin(num='35',name='35',func=pin_types.UNSPEC),
            Pin(num='36',name='36',func=pin_types.UNSPEC),
            Pin(num='37',name='37',func=pin_types.UNSPEC),
            Pin(num='38',name='38',func=pin_types.UNSPEC),
            Pin(num='39',name='39',func=pin_types.UNSPEC),
            Pin(num='40',name='40',func=pin_types.UNSPEC)] }),
        Part(**{ 'name':'BARREL_5V', 'dest':TEMPLATE, 'tool':SKIDL, 'aliases':Alias({'BARREL_5V'}), 'ref_prefix':'U', 'fplist':None, 'footprint':'Connector_BarrelJack:BarrelJack_CUI_PJ-063AH_Horizontal', 'keywords':None, 'description':'', 'datasheet':None, 'pins':[
            Pin(num='1',name='1',func=pin_types.UNSPEC),
            Pin(num='2',name='2',func=pin_types.UNSPEC),
            Pin(num='MP',name='MP',func=pin_types.UNSPEC)] }),
        Part(**{ 'name':'J_AV', 'dest':TEMPLATE, 'tool':SKIDL, 'aliases':Alias({'J_AV'}), 'ref_prefix':'U', 'fplist':None, 'footprint':'Connector_PinHeader_2.54mm:PinHeader_2x04_P2.54mm_Vertical', 'keywords':None, 'description':'', 'datasheet':None, 'pins':[
            Pin(num='1',name='1',func=pin_types.UNSPEC),
            Pin(num='2',name='2',func=pin_types.UNSPEC),
            Pin(num='3',name='3',func=pin_types.UNSPEC),
            Pin(num='4',name='4',func=pin_types.UNSPEC),
            Pin(num='5',name='5',func=pin_types.UNSPEC),
            Pin(num='6',name='6',func=pin_types.UNSPEC),
            Pin(num='7',name='7',func=pin_types.UNSPEC),
            Pin(num='8',name='8',func=pin_types.UNSPEC)] }),
        Part(**{ 'name':'J_PWR', 'dest':TEMPLATE, 'tool':SKIDL, 'aliases':Alias({'J_PWR'}), 'ref_prefix':'U', 'fplist':None, 'footprint':'Connector_PinHeader_2.54mm:PinHeader_2x02_P2.54mm_Vertical', 'keywords':None, 'description':'', 'datasheet':None, 'pins':[
            Pin(num='1',name='1',func=pin_types.UNSPEC),
            Pin(num='2',name='2',func=pin_types.UNSPEC),
            Pin(num='3',name='3',func=pin_types.UNSPEC),
            Pin(num='4',name='4',func=pin_types.UNSPEC)] }),
        Part(**{ 'name':'J_ISP', 'dest':TEMPLATE, 'tool':SKIDL, 'aliases':Alias({'J_ISP'}), 'ref_prefix':'U', 'fplist':None, 'footprint':'Connector_PinHeader_2.54mm:PinHeader_2x03_P2.54mm_Vertical', 'keywords':None, 'description':'', 'datasheet':None, 'pins':[
            Pin(num='1',name='1',func=pin_types.UNSPEC),
            Pin(num='2',name='2',func=pin_types.UNSPEC),
            Pin(num='3',name='3',func=pin_types.UNSPEC),
            Pin(num='4',name='4',func=pin_types.UNSPEC),
            Pin(num='5',name='5',func=pin_types.UNSPEC),
            Pin(num='6',name='6',func=pin_types.UNSPEC)] }),
        Part(**{ 'name':'XTAL_20M', 'dest':TEMPLATE, 'tool':SKIDL, 'aliases':Alias({'XTAL_20M'}), 'ref_prefix':'U', 'fplist':None, 'footprint':'Crystal:Crystal_HC49-U_Vertical', 'keywords':None, 'description':'', 'datasheet':None, 'pins':[
            Pin(num='1',name='1',func=pin_types.UNSPEC),
            Pin(num='2',name='2',func=pin_types.UNSPEC)] }),
        Part(**{ 'name':'SCHOTTKY', 'dest':TEMPLATE, 'tool':SKIDL, 'aliases':Alias({'SCHOTTKY'}), 'ref_prefix':'U', 'fplist':None, 'footprint':'Diode_THT:D_DO-41_SOD81_P2.54mm_Vertical_CathodeUp', 'keywords':None, 'description':'', 'datasheet':None, 'pins':[
            Pin(num='1',name='1',func=pin_types.UNSPEC),
            Pin(num='2',name='2',func=pin_types.UNSPEC)] }),
        Part(**{ 'name':'C_BULK', 'dest':TEMPLATE, 'tool':SKIDL, 'aliases':Alias({'C_BULK'}), 'ref_prefix':'U', 'fplist':None, 'footprint':'Capacitor_THT:CP_Radial_D8.0mm_P3.50mm', 'keywords':None, 'description':'', 'datasheet':None, 'pins':[
            Pin(num='1',name='1',func=pin_types.UNSPEC),
            Pin(num='2',name='2',func=pin_types.UNSPEC)] }),
        Part(**{ 'name':'C_22P', 'dest':TEMPLATE, 'tool':SKIDL, 'aliases':Alias({'C_22P'}), 'ref_prefix':'U', 'fplist':None, 'footprint':'Capacitor_THT:C_Disc_D5.0mm_W2.5mm_P5.00mm', 'keywords':None, 'description':'', 'datasheet':None, 'pins':[
            Pin(num='1',name='1',func=pin_types.UNSPEC),
            Pin(num='2',name='2',func=pin_types.UNSPEC)] }),
        Part(**{ 'name':'C_100N', 'dest':TEMPLATE, 'tool':SKIDL, 'aliases':Alias({'C_100N'}), 'ref_prefix':'U', 'fplist':None, 'footprint':'Capacitor_THT:C_Disc_D5.0mm_W2.5mm_P5.00mm', 'keywords':None, 'description':'', 'datasheet':None, 'pins':[
            Pin(num='1',name='1',func=pin_types.UNSPEC),
            Pin(num='2',name='2',func=pin_types.UNSPEC)] }),
        Part(**{ 'name':'R_10K', 'dest':TEMPLATE, 'tool':SKIDL, 'aliases':Alias({'R_10K'}), 'ref_prefix':'U', 'fplist':None, 'footprint':'Resistor_THT:R_Axial_DIN0207_L6.3mm_D2.5mm_P2.54mm_Vertical', 'keywords':None, 'description':'', 'datasheet':None, 'pins':[
            Pin(num='1',name='1',func=pin_types.UNSPEC),
            Pin(num='2',name='2',func=pin_types.UNSPEC)] }),
        Part(**{ 'name':'R_1K', 'dest':TEMPLATE, 'tool':SKIDL, 'aliases':Alias({'R_1K'}), 'ref_prefix':'U', 'fplist':None, 'footprint':'Resistor_THT:R_Axial_DIN0207_L6.3mm_D2.5mm_P2.54mm_Vertical', 'keywords':None, 'description':'', 'datasheet':None, 'pins':[
            Pin(num='1',name='1',func=pin_types.UNSPEC),
            Pin(num='2',name='2',func=pin_types.UNSPEC)] }),
        Part(**{ 'name':'R_470', 'dest':TEMPLATE, 'tool':SKIDL, 'aliases':Alias({'R_470'}), 'ref_prefix':'U', 'fplist':None, 'footprint':'Resistor_THT:R_Axial_DIN0207_L6.3mm_D2.5mm_P2.54mm_Vertical', 'keywords':None, 'description':'', 'datasheet':None, 'pins':[
            Pin(num='1',name='1',func=pin_types.UNSPEC),
            Pin(num='2',name='2',func=pin_types.UNSPEC)] }),
        Part(**{ 'name':'R_75', 'dest':TEMPLATE, 'tool':SKIDL, 'aliases':Alias({'R_75'}), 'ref_prefix':'U', 'fplist':None, 'footprint':'Resistor_THT:R_Axial_DIN0207_L6.3mm_D2.5mm_P2.54mm_Vertical', 'keywords':None, 'description':'', 'datasheet':None, 'pins':[
            Pin(num='1',name='1',func=pin_types.UNSPEC),
            Pin(num='2',name='2',func=pin_types.UNSPEC)] }),
        Part(**{ 'name':'R_0', 'dest':TEMPLATE, 'tool':SKIDL, 'aliases':Alias({'R_0'}), 'ref_prefix':'U', 'fplist':None, 'footprint':'Resistor_THT:R_Axial_DIN0207_L6.3mm_D2.5mm_P2.54mm_Vertical', 'keywords':None, 'description':'', 'datasheet':None, 'pins':[
            Pin(num='1',name='1',func=pin_types.UNSPEC),
            Pin(num='2',name='2',func=pin_types.UNSPEC)] })])