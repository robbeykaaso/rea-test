#include <QtSerialBus/QModbusClient>
#include <QtSerialPort/QSerialPort>
#include "reaC++.h"
#include "modbusMaster.h"

static rea::regPip<QJsonObject> test_modbus([](rea::stream<QJsonObject>* aInput){
    if (!aInput->data().value("modbus").toBool()){
        aInput->out();
        return;
    }

    const QJsonObject testBus = rea::Json(QString::number(QModbusDevice::SerialPortNameParameter), "COM2",
                                          QString::number(QModbusDevice::SerialParityParameter), QSerialPort::Parity::EvenParity,
                                          QString::number(QModbusDevice::SerialBaudRateParameter), QSerialPort::Baud19200,
                                          QString::number(QModbusDevice::SerialDataBitsParameter), QSerialPort::DataBits::Data8,
                                          QString::number(QModbusDevice::SerialStopBitsParameter), QSerialPort::StopBits::OneStop);

    static rea::modBusMaster modbus(testBus);
    //QLoggingCategory::setFilterRules(QStringLiteral("qt.modbus* = true"));

    rea::pipeline::find("callSlave")
        ->next(rea::pipeline::add<QByteArray>([](rea::stream<QByteArray>* aInput){
                   auto dt = aInput->data();
                   assert(dt == "");
                   //assert(dt == "0100");
                   aInput->outs<QString>("Pass: testModbusMaster ", "testSuccess");
               }), "testModBus")
        ->next("testSuccess");

    rea::pipeline::instance()->run<QJsonObject>("callSlave", rea::Json("func", QModbusRequest::FunctionCode::ReadCoils,
                                                           "payload", "00000001"), "testModBus");

    aInput->out();
}, QJsonObject(), "unitTest");
