screen 12
cls
print "Welcome to polynomials beta."
degree:
dim n as integer
n=0
ds=0
de=0
fde=0
fds=0
input "Enter polynomial degree";n
if n<0 then
print "The program solves polynomials only."
goto degree
end if
dim degrees(n+1)
coefficients:
for i=0 to n
l=n-i
if l=0 then
print "Enter free term"
elseif l=1  then
print "Enter coefficient of x"
else
print "Enter coefficeint of x^";l
end if
input degrees(i)
if degrees(0)=0 then
print"Coefficient of x^";n;" can't be equal zero"
goto coefficients
end if
next i
cls
domain:
input "Enter domain start";ds!
input "Enter domain end";de!
cls
if ds=de then
print "That is an invalid domain."
cls
goto domain
end if
cls
if ds>de then
swap ds,de
end if
for i=0 to n
l=n-i
fds=fds+degrees(i)*(ds+10^-100)^l
fde=fde+degrees(i)*(de+10^-100)^l
next i
if abs(fde)>10^308 then
print"The program can't deal with infinities."
erase degrees
goto degree
elseif abs(fds)>10^308 then
print"The program can't deal with infinities."
erase degrees
goto degree
elseif fds=0 and fde=0 then
print"The domain returns nearly equal values at y."
erase degrees
goto degree
end if
if abs(de)>abs(ds) then
window(-abs(de),-abs(fde))-(abs(de),abs(fde))
line(-abs(de),0)-(abs(de),0),15
line(0,-abs(fde))-(0,abs(fde)),15
print "x-axis ranges from ";-abs(de);"to ";abs(de)
print "y-axis ranges from ";-abs(fde);"to ";abs(fde)
else
window(-abs(ds),-abs(fds))-(abs(ds),abs(fds))
line(-abs(ds),0)-(abs(ds),0),15
line(0,-abs(fds))-(0,abs(fds)),15
print "x-axis ranges from ";-abs(ds);"to ";abs(ds)
print "y-axis ranges from ";-abs(fds);"to ";abs(fds)
end if
stp=abs(de-ds)/100000
graph:
for x=ds to de step stp
y=0
for i=0 to n
l=n-i
y=y+degrees(i)*(x+10^-100)^l
next i
pset(x,y),4
next x
for x=ds to de step stp
fxi=0
for i=0 to n
l=n-i
fxi=fxi+degrees(i)/(l+1)*(x+10^-200)^(l+1)
next i
pset(x,fxi),2
next x
print "Green:Integration with constant=0"
for x=ds to de step stp
dx=0
for i=0 to n
l=n-i
dx=dx+degrees(i)*l*(x+10^-340)^(l-1)
next i
pset(x,dx),1
next x
print"Red:Function"
print "Blue:Derivative function"
tangency:
if n=1 then
goto integration1
elseif n=0 then
goto integration1
goto ending
else
input "Enter the x of the point of tangency";t!
if t<ds then
print "That is out of domain."
goto tangency
elseif t= ds then
print "That is not differentiable."
goto tangency
elseif t>de then
print "That is out of domain."
goto tangency
elseif t=de then
print "That is not differentiable."
goto tangency
end if
dt=0
ft=0
t1=0
t2=0
for i=0 to n
l=n-i
t1=t1+degrees(i)*(t-1)^l
t2=t2+degrees(i)*(t+1)^l
ft=ft+degrees(i)*(t+10^-100)^l
dt=dt+degrees(i)*l*(t+10^-100)^(l-1)
next i
if t1=-t2 then
print "No tangent as it is not differentiable."
goto tangency
elseif dt=0 then
print "The slope of the tangent at (";t;",";ft;") is 0"
if abs(de)>abs(ds) then
line (-abs(de),ft)-(abs(de),ft),6
else
line(-abs(ds),ft)-(abs(ds),ft),6
end if
else
print "The slope of the tangent at (";t;",";ft;") is ";dt
if abs(de)>abs(ds) then
for x=-abs(de) to abs(de) step stp
pset(x,dt*x-dt*t+ft),6
next x
else
for x=-abs(ds) to abs(ds) step stp
pset(x,dt*x-dt*t+ft),6
next x
end if
end if
end if
integration1:
iie=0
iis=0
input "Enter lower limit of integration.";is!
if is<ds then
print "That is out of domain."
goto integration1:
elseif is>de then
print "That is out of domain."
goto integration1:
end if
integration2:
input "Enter upper limit of integration.";ie!
if ie>de then
print "That is out of domain."
goto integration2
elseif ie<ds then
print "That is out of domain."
goto integration2
end if
if ie=is then
elseif ie>is then
for x=is to ie step stp
ix=0
for i=0 to n
l=n-i
ix=ix+degrees(i)*x^l
next i
line(x,ix)-(x+0.001,0),14
next x
else
for x=ie to is step stp
ix=0
for i=0 to n
l=n-i
ix=ix+degrees(i)*x^l
next i
line(x,ix)-(x+0.001,0),14
next x
end if
for i=0 to n
l=n-i
iis=iis+degrees(i)/(l+1)*is^(l+1)
iie=iie+degrees(i)/(l+1)*ie^(l+1)
next i
print "The definite integral between the two limits = ";iie-iis
if n=1 then
print "The line root is x= ";-degrees(1)/degrees(0)
end if
if n=2 then
fc=0
for i=0 to n
l=n-i
fc=fc+degrees(i)*(-degrees(1)/(2*degrees(0))+10^-100)^l
next i
print "Critical point at (";-degrees(1)/(2*degrees(0));",";fc;")"
a=degrees(0)
b=degrees(1)
c=degrees(2)
disc=b^2-4*a*c
if disc=0 then
print "The function has one real root."
print "x= ";-b/(2*a)
elseif disc>0 then
print "The function roots are real."
print "x= ";(-b-sqr(disc))/(2*a)
print "x= ";(-b+sqr(disc))/(2*a)
else
print "The function roots are imaginary "
print "x= ";-b/(2*a);"-";sqr(-disc)/(2*a);"i"
print "x= ";-b/(2*a);"+";sqr(-disc)/(2*a);"i"
end if
end if
fb=0
if n=3 then
for i=0 to n
l=n-i
fb=fb+degrees(i)*(-((degrees(1))/(3*degrees(0)))+10^-100)^l
next i
print "Inflection point at (";-(degrees(1))/(3*degrees(0));",";fb;")"
end if
for x=ds to de step stp
y=0
for i=0 to n
l=n-i
y=y+degrees(i)*(x+10^-100)^l
next i
pset(x,y),4
next x
choice:
dim r as string
input "To try another one press y to exit press n";r
if r="y" then
erase degrees
cls
goto degree
elseif r="n" then
print"Thanks!"
goto ending
else
goto choice
end if
ending:
