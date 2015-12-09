void print_glcd_setup()
{
 #ifdef DEBUG
  Serial.println("emonGLCD solar PV monitor - gen and use");
  Serial.println("openenergymonitor.org");
  Serial.print("Node: "); 
  Serial.print(MYNODE); 
  Serial.print(" Freq: "); 
  Serial.print("868Mhz");
  Serial.print(" Network: "); 
  Serial.println(group);
 #endif
}

void print_emontx_payload()
{
 #ifdef DEBUG
  Serial.println();
  Serial.print("emontx: ");
  Serial.print(emontx.power1);
  Serial.print(' ');
  Serial.print(emontx.power2);
  Serial.print(' ');
  Serial.println(emontx.Vrms);
 #endif
}

void print_baseNode_payload()
{
 #ifdef DEBUG
  Serial.print("baseNode ID: ");
  Serial.print(baseNode.NodeType);
  Serial.print(" - ");
  Serial.print(baseNode.hour);
  Serial.print(":");
  Serial.print(baseNode.mins);
  Serial.print(":");
  Serial.print(baseNode.sec);
  Serial.print(" | Temp: ");
  Serial.print(baseNode.temperature);
  Serial.print(" | LCD: ");
  Serial.println(baseNode.BackLight);
 #endif
}
