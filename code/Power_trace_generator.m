f = 68e6; %global
t = 0:1:12000;
t = t.*1.5e-9;
p = 10*cos(2*3.1415924*f*t);
p = abs(p);

p = p';


core1 = 7.*[p p p p p p]./6;
core2 = 7.*[p p p p p p]./6;
core3 = 7.*[p p p p p p]./6;
core4 = 7.*[p p p p p p]./6;





P = [core1 core2 core3 core4];




csvwrite('GPU_Power.txt',P)