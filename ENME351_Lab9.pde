import processing.serial.*; // add the serial library
Serial myPort; // define a serial port object to monitor

int w = 1200;
int h = w/2;

float handAngle = PI/2.0;
float degreesPerStep = 2.0*PI/516.0;
int clockDiameter = h/3;
int clockX = w/2+w/12;
int clockY = h/2;

int rectWidth = w/12*5;
int rectHeight = w/3;
int rectX = int(w/2 -.46*w);
int rectY = h/2 - rectHeight/2;

int armLength = w/4;
float L1 = 132*w/1000;
float L2 = 131.5*w/1000;
int lineX = rectX+rectWidth*3/4;
int lineY = rectY+rectHeight*3/4;

int counter = 0;

void setup() {
  size(1200, 600); // set the window size
  background(211, 211, 211);
  println(Serial.list()); // list all available serial ports
  myPort = new Serial(this, Serial.list()[0], 9600); // define input port
  myPort.clear(); // clear the port of any initial junk
  
  stroke(0, 0, 0);
  strokeWeight(2);
  fill(211, 211, 211);
  
  //Drawing clock diagram
  ellipse(clockX, clockY, clockDiameter, clockDiameter);
  line(clockX, clockY-clockDiameter/2, clockX, clockY-clockDiameter/2*.9); //Cardinal Lines
  line(clockX, clockY+clockDiameter/2, clockX, clockY+clockDiameter/2*.9);
  line(clockX-clockDiameter/2, clockY, clockX-clockDiameter/2*.9, clockY);
  line(clockX+clockDiameter/2, clockY, clockX+clockDiameter/2*.9, clockY);
  line(clockX-1/sqrt(2)*clockDiameter/2, clockY-1/sqrt(2)*clockDiameter/2, clockX-1/sqrt(2)*clockDiameter/2*.9, clockY-1/sqrt(2)*clockDiameter/2*.9); //Diagonal lines
  line(clockX-1/sqrt(2)*clockDiameter/2, clockY+1/sqrt(2)*clockDiameter/2, clockX-1/sqrt(2)*clockDiameter/2*.9, clockY+1/sqrt(2)*clockDiameter/2*.9);
  line(clockX+1/sqrt(2)*clockDiameter/2, clockY-1/sqrt(2)*clockDiameter/2, clockX+1/sqrt(2)*clockDiameter/2*.9, clockY-1/sqrt(2)*clockDiameter/2*.9);
  line(clockX+1/sqrt(2)*clockDiameter/2, clockY+1/sqrt(2)*clockDiameter/2, clockX+1/sqrt(2)*clockDiameter/2*.9, clockY+1/sqrt(2)*clockDiameter/2*.9);
  
  frameRate(120);
}

void draw () {

  while (myPort.available() > 0) { // make sure port is open
    String inString = myPort.readStringUntil('\n'); // read input string

    if (inString != null) { // ignore null strings
      inString = trim(inString); // trim off any whitespace
      
      //xCoord yCoord alpha beta epsilon stepperDirection laserState --> 7 elements
      String[] data = splitTokens(inString, " "); // extract x & y into an array

      if (data.length == 8) {
        //Extracting data from String parse
        float xCoord = float(data[0]);
        float yCoord = float(data[1]);
        float alpha = float(data[2])/180*PI;
        float beta = float(data[3])/180*PI;
        float epsilon = float(data[4]);
        float stepperDirection = float(data[5]);
        float laserState = float(data[6]);
        float photoState = float(data[7]);
        
        println(counter);
        counter++;
        
        fill(211, 211, 211);
        if (stepperDirection > 0) {
          handAngle += degreesPerStep;
          drawClock();
        } else if (stepperDirection < 0) {
          handAngle -= degreesPerStep; 
          drawClock();
        }
        
        drawProfile(xCoord, yCoord, alpha, beta);
        drawLaser(laserState);
      }
    }
  }
}

void drawClock() {
  //Drawing over previous clock hand
  stroke(211, 211, 211);
  strokeWeight(6);
  line(clockX, clockY, clockX-clockDiameter/2*.8*cos(handAngle), clockY-clockDiameter/2*.8*sin(handAngle));
  rect(clockX-clockDiameter*.38, clockY - clockDiameter*1.5, clockDiameter, clockDiameter*.95);
  
  //Drawing new clock hand
  stroke(0, 0, 0);
  fill(0);
  text(handAngle/PI*180.0, clockX-clockDiameter*.38, clockY - clockDiameter*.54);
  fill(211, 211, 211);
  strokeWeight(2);
  line(clockX, clockY, clockX-clockDiameter/2*.7*cos(handAngle), clockY-clockDiameter/2*.7*sin(handAngle));
}

void drawProfile(float xCoord, float yCoord, float alpha, float beta) {
  stroke(0, 0, 0);
  rect(rectX, rectY, rectWidth, rectHeight);
  line(lineX, lineY, lineX-L2*cos(alpha), lineY-L2*sin(alpha));
  line(lineX-L2*cos(alpha), lineY-L2*sin(alpha), lineX-L2*cos(alpha)-L1*cos(alpha-beta), lineY-L2*sin(alpha)-L1*sin(alpha-beta));
  
  stroke(211, 211, 211);
  rect(rectX+w*.016, rectY-h*.08, rectWidth, rectHeight*.1);
  fill(0);
  textSize(w/25);
  text("X:", rectX+w*.016, rectY-h*.016);
  text(xCoord, rectX+w*.04, rectY-h*.016);
  text("Y:", rectX+rectWidth-w*.137, rectY-h*.016);
  text(yCoord, rectX+rectWidth-w*.114, rectY-h*.016);
  stroke(0, 0, 0);
}

void drawLaser(float laserState) {
  stroke(0, 0, 0);
  strokeWeight(1);
  fill(225);
  ellipse(.85*w, h/2, clockDiameter, clockDiameter);
  
  if (laserState == 1) {
    stroke(225);
    fill(0, 0, 255);
    ellipse(.85*w, .5*h, clockDiameter/4, clockDiameter/4);
  } else {
    stroke(225);
    fill(0, 0, 0);
    ellipse(.85*w, .5*h, clockDiameter/4, clockDiameter/4);
  }
}
