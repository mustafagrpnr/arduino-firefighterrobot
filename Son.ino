#include <AFMotor.h>
#include <Servo.h>

//Motor
AF_DCMotor motorlb(1);
AF_DCMotor motorrb(2);
AF_DCMotor motorrf(3);
AF_DCMotor motorlf(4);

//Ateş Sensörü Pin
int flame1 = A4;
int flame2 = A2;
int flame3 = A3;

//Ateş Sensörü Sonuç Değişkenleri
short res1 = 0;
short res2 = 0;
short res3 = 0;

//Servo - Servo açı değişkenleri
Servo servo;
int i=0;

//Diğer değişkenler
short detected = 0;
short detServoAngle = 0;
short detFlameNum = 0;
short lastMainSensorAngle = 0;
short mainSensorAngle = 164;

//Motor Fonksiyonları
//İleri git
void mForward(){
  motorrb.run(BACKWARD);
  motorrf.run(BACKWARD);
  motorlb.run(BACKWARD);
  motorlf.run(BACKWARD);
}
//Geri git
void mBackward(){
  motorrb.run(FORWARD);
  motorrf.run(FORWARD);
  motorlb.run(FORWARD);
  motorlf.run(FORWARD);
}
//Sola dön
void mLeft(){
  motorrb.run(BACKWARD);
  motorrf.run(BACKWARD);
  motorlf.run(RELEASE);
  motorlb.run(RELEASE);
}
//Sağa dön
void mRight(){
  motorlb.run(BACKWARD);
  motorlf.run(BACKWARD);
  motorrf.run(RELEASE);
  motorrb.run(RELEASE);
}
//Sola geri gideriek dön
void mLeftBack(){
  motorlb.run(FORWARD);
  motorlf.run(FORWARD);
  motorrf.run(RELEASE);
  motorrb.run(RELEASE);
}
//Sağa geri giderek dön
void mRightBack(){
  motorrb.run(FORWARD);
  motorrf.run(FORWARD);
  motorlf.run(RELEASE);
  motorlb.run(RELEASE);
}
//Hızı ayarla (Max:255)
//
//-- Parametreler -- 
//mSpeed = Motor hızı
void mSetSpeed(short mSpeed){
  motorlb.setSpeed(mSpeed);
  motorrb.setSpeed(mSpeed);
  motorrf.setSpeed(mSpeed);
  motorlf.setSpeed(mSpeed);
}
//Dur
void mRelease(){
  motorlb.run(RELEASE);
  motorrb.run(RELEASE);
  motorrf.run(RELEASE);
  motorlf.run(RELEASE);
}

//Servoyu belirlenen açılar arasında ardışıl zıt hareket ettir
//
//-- Parametreler -- 
//sStart = Başlangıç açısı
//sStop = Bitiş Açısı
//inc = Her loopta kaç derece artırılması gerektiği
short servoAngle(short sStart, short sStop, short inc){
  short sDif = sStop-sStart;
  if(i%(sDif*2)>(sDif-1)){
    servo.write(sStop-(i%sDif));
    i+=inc;
    return (sStop-(i%sDif));
  }else{
    servo.write(sStart+(i%sDif));
    i+=inc;
    return (sStart+(i%sDif));
  }
}

//Eğer ateş tespit edilirse
//
//Ana sensöz numarası = 3
//Ana sensör servo açısı = 164
//Sensör 1'in servo açısı 111'den büyükse ateş SAĞda
//Sensör 2'nin servo açısı 47'den küçükse ateş SAĞda
//Sensör 3'ün servo açısı 164'den küçükse ateş SAĞda
void fDetected(){
  if(res3<650){
    detected = 2;
  }
  //Eğer arabanın yönü ateşe dönmüşse
  if(detected==2){
    //Eğer araba ateşi söndürebilcek kadar yakınsa
    if(res3<30){
      //Eğer açı tam doğru değilse bir miktar dön (Ana sensörün sapması kadar dön)
      if(lastMainSensorAngle < mainSensorAngle){
        mRightBack();
        delay((mainSensorAngle-lastMainSensorAngle)*15);
        mRight();
        delay((mainSensorAngle-lastMainSensorAngle)*15);
      }else if(lastMainSensorAngle > mainSensorAngle){
        mLeftBack();
        delay((lastMainSensorAngle-mainSensorAngle)*15);
        mLeft();
        delay((lastMainSensorAngle-mainSensorAngle)*15);
      }
      //Gerekli değişkenleri sıfırla. Arabayı durdur. Pervaneyi çalıştır 1 sn bekle.
      i = 0;
      detected = 0;
      mRelease();
      analogWrite(A5,0);
      delay(1000);
    }else{ //Eğer araba ateşe yakın değilse ön kısmı tarayarak ilerle. Hızı bir miktar yavaşlat
      lastMainSensorAngle = servoAngle(129,199,3);
      mSetSpeed(200);
      mForward();
    }
  }else{ 
    //Eğer etrafta ateş tespit edilmiş fakat arabanın yönü o yöne doğru değilse arabayı döndür
    if(detFlameNum==1&&detServoAngle>111 || 
       detFlameNum==2&&detServoAngle<47 || 
       detFlameNum==3&&detServoAngle>164
      ){
      mRight();
    }else{
      mLeft();
    }
  }
}

void setup() {
  mSetSpeed(255);
  mRelease();
  
  servo.attach(9);
  pinMode(OUTPUT,2);
  Serial.begin(9600);
}
void loop() {
  //Sensör değerlerini oku
  res1 = analogRead(flame1);
  res2 = analogRead(flame2);
  res3 = analogRead(flame3);

  //Ateş tespit edilirse
  if(detected>=1){
    fDetected();
  }else{ //Eğer ateş tespit edilmemişse
    // Hızı yükselt
    mSetSpeed(255);
    //Eğer ilk işlem değilse ilerle
    if(i>360){
      mForward();
    }
    //Servoyu döndürerek etrafı tara
    servoAngle(0,180,2);
    //Eğer etrafı tararken ateş tespit edilirse gerekli değerleri değişkenlere ata.
    //Servoyu ana servo açısına döndür
    //Tespit edildiği için detected=1 oldu. Bir sonraki loopta ilk ifte then kısmı çalışcak
    if(res1<600){
      lastMainSensorAngle = mainSensorAngle;
      servo.write(lastMainSensorAngle);
      detected = 1;
      detFlameNum = 1;
      detServoAngle = i;
      delay(100);
    }else if(res2<600){
      lastMainSensorAngle = mainSensorAngle;
      servo.write(lastMainSensorAngle);
      detected = 1;
      detFlameNum = 2;
      detServoAngle = i;
      delay(100);
    }else if(res3<600){
      lastMainSensorAngle = mainSensorAngle;
      servo.write(lastMainSensorAngle);
      detected = 1;
      detFlameNum = 3;
      detServoAngle = i;
      delay(100);
    }
  }
  delay(15);
}
