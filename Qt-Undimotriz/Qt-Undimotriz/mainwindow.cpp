#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "pressureconverter.h"
#include <QTcpSocket>
#include <QByteArray>
#include <QMessageBox>
#include <QTimer>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <cmath>  // Para atan2() y M_PI
// #include <QDebug>  // Comentado para producción
#include <QPushButton>
#include <QLineEdit>

namespace {
constexpr int PERIODO_MUESTREO_MS = 5;
constexpr int MAX_MUESTRAS_MEDICION = 60000;
}


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    
    // Establecer icono de la aplicación
    setWindowIcon(QIcon(":/Undimotriz.png"));

    tcpSocket = nullptr;
    
    // Inicializar variables del protocolo de 15 bytes fijos
    estadoRecepcion = ESPERO_INICIO_TRAMA;
    contadorBytes = 0;
    tramaBuffer.reserve(TRAMA_TOTAL_BYTES); // 15 bytes
    
    // Inicializar variables para LCD (igual que ESP32)
    tensionFiltrada = 0.0;
    corrienteFiltrada = 0.0;
    potenciaCalculada = 0.0;
    datosValidos = false;
    
    // Inicializar nuevos sensores
    alturaFiltrada = 0.0;
    rpmActual = 0;
    aceleracionX = 0.0;
    aceleracionY = 0.0;
    
    // Inicializar estados de cargas (todos desactivados)
    estadoRele0 = false;  // CARGA 1 (pushButtonR1)
    estadoRele1 = false;  // CARGA 2 (pushButtonR2)
    estadoRele2 = false;  // CARGA 3 (pushButtonR3)
    estadoRele3 = false;  // CARGA 4 (pushButtonR4)
    alertaSobretensionActiva = false;

    
    // Inicializar variables para captura (inactivas por defecto)
    capturandoMediciones = false;
    datosMediciones.clear();
    tiempoInicioMediciones = 0;

    // Configurar timer para actualización LCD cada 1 segundo (igual que ESP32)
    connect(&lcdTimer, &QTimer::timeout, this, &MainWindow::actualizarLCD);
    lcdTimer.start(1000); // 1000ms = 1 segundo

    // Notificaciones internas de alerta por sobretensión
    connect(this, &MainWindow::sobretensionActivada, this, &MainWindow::onSobretensionActivada);
    connect(this, &MainWindow::sobretensionDesactivada, this, &MainWindow::onSobretensionDesactivada);
    
    // Configurar QLineEdit del LCD como solo lectura
    ui->edit_lcd_1->setReadOnly(true);
    ui->edit_lcd_2->setReadOnly(true);
    ui->edit_lcd_3->setReadOnly(true);
    ui->edit_lcd_4->setReadOnly(true);
    
    // Configurar QLineEdit de las cargas como solo lectura y centrados
    ui->lineEditR1->setReadOnly(true);
    ui->lineEditR1->setAlignment(Qt::AlignCenter);
    ui->lineEditR1->setFocusPolicy(Qt::NoFocus);
    ui->lineEditR2->setReadOnly(true);
    ui->lineEditR2->setAlignment(Qt::AlignCenter);
    ui->lineEditR2->setFocusPolicy(Qt::NoFocus);
    ui->lineEditR3->setReadOnly(true);
    ui->lineEditR3->setAlignment(Qt::AlignCenter);
    ui->lineEditR3->setFocusPolicy(Qt::NoFocus);
    ui->lineEditR4->setReadOnly(true);
    ui->lineEditR4->setAlignment(Qt::AlignCenter);
    ui->lineEditR4->setFocusPolicy(Qt::NoFocus);

    // Inicializar apariencia de botones de cargas
    actualizarEstadoBotonRele(ui->pushButtonR1, estadoRele0);  // CARGA 1
    actualizarEstadoBotonRele(ui->pushButtonR2, estadoRele1);  // CARGA 2
    actualizarEstadoBotonRele(ui->pushButtonR3, estadoRele2);  // CARGA 3
    actualizarEstadoBotonRele(ui->pushButtonR4, estadoRele3);  // CARGA 4
    
    
    // Inicializar área de mediciones deshabilitada (sin conexión)
    habilitarAreaMediciones(false);
    
    // Configurar botón de medición (toggle). Solo texto, no cambiar estilo global
    ui->pushButtonMedicion->setCheckable(true);
    ui->pushButtonMedicion->setText("Iniciar Medicion");

    // Ocultar completamente la pestaña Log para el usuario final
    // El usuario solo debe ver la pestaña principal de mediciones
    ui->tabWidget->removeTab(1);  // Remover pestaña Log (índice 1)
    //Alternativa: ui->tabWidget->setTabEnabled(1, false);  // Solo deshabilitar
    
    actualizarEstadoConexion();
}

MainWindow::~MainWindow()
{
    delete ui;
    if (tcpSocket != nullptr) {
        tcpSocket->disconnectFromHost();
        delete tcpSocket;
    }
}



void MainWindow::actualizarEstadoConexion()
{
    if (conectado()) {
        if (alertaSobretensionActiva) {
            ui->lineEditEstado->setStyleSheet("font-weight: bold; color: black; background-color: yellow;");
            ui->lineEditEstado->setText("ALERTA SOBRETENSION");
        } else {
            ui->lineEditEstado->setStyleSheet("font-weight: bold; color: black; background-color: lightgreen;");
            ui->lineEditEstado->setText("CONECTADO");
        }
        ui->pushButtonConectar->setText("Desconectar");
        
        // Habilitar área de mediciones cuando esté conectado
        habilitarAreaMediciones(true);
    } else {
        ui->lineEditEstado->setStyleSheet("font-weight: normal; color: white; background-color: red;");
        ui->lineEditEstado->setText("Desconectado");
        ui->pushButtonConectar->setText("Conectar");
        
        // Deshabilitar área de mediciones cuando esté desconectado
        habilitarAreaMediciones(false);
    }
}

bool MainWindow::conectado()
{
    if (tcpSocket != nullptr) {
        return (tcpSocket->state() == QAbstractSocket::ConnectedState);
    } else
        return false;
}

bool MainWindow::parsearDireccionServidor(const QString &address, QString &host, quint16 &port)
{
    QStringList parts = address.split(':');
    if (parts.size() != 2) {
        return false;
    }
    
    host = parts[0].trimmed();
    bool ok;
    port = parts[1].trimmed().toUShort(&ok);
    
    return ok && !host.isEmpty() && port > 0;
}

void MainWindow::procesarTramaTelemetria()
{
    // ========================================================================
    // EXTRACCIÓN DE DATOS RAW (formato MSB,LSB)
    // ========================================================================
    // Tensión ADC (0-1023)
    uint16_t adc_tension = ((uint8_t)tramaBuffer[IDX_TENSION_MSB] << 8) | 
                           (uint8_t)tramaBuffer[IDX_TENSION_LSB];

    uint16_t adc_corriente = ((uint8_t)tramaBuffer[IDX_CORRIENTE_MSB] << 8) | 
                             (uint8_t)tramaBuffer[IDX_CORRIENTE_LSB];


    // Presión ADC (0-1023)
    uint16_t adc_presion = ((uint8_t)tramaBuffer[IDX_PRESION_MSB] << 8) | 
                           (uint8_t)tramaBuffer[IDX_PRESION_LSB];
    qDebug()<< adc_presion;
    // RPM (0-600, ya convertido por Arduino)
    uint16_t rpmNuevo = ((uint8_t)tramaBuffer[IDX_RPM_MSB] << 8) | 
                        (uint8_t)tramaBuffer[IDX_RPM_LSB];
    if (rpmNuevo <= 600) {
        rpmActual = rpmNuevo;
    }
    
    // Aceleración X (con signo, ±32768)
    int16_t accel_x_raw = ((uint8_t)tramaBuffer[IDX_ACCEL_X_MSB] << 8) | 
                          (uint8_t)tramaBuffer[IDX_ACCEL_X_LSB];
    
    // Aceleración Y (con signo, ±32768)
    int16_t accel_y_raw = ((uint8_t)tramaBuffer[IDX_ACCEL_Y_MSB] << 8) | 
                          (uint8_t)tramaBuffer[IDX_ACCEL_Y_LSB];
    
    // Estado digital RX: bits 0-3 relés, bit 7 alerta por sobretensión
    uint8_t estado_digital = (uint8_t)tramaBuffer[IDX_ESTADOS];
    bool sobretensionActivaRx = (estado_digital & 0x80) != 0;
    uint8_t estado_reles = estado_digital & 0x0F;

    if (sobretensionActivaRx != alertaSobretensionActiva) {
        alertaSobretensionActiva = sobretensionActivaRx;
        emit sobretensionCambiada(alertaSobretensionActiva);
        if (alertaSobretensionActiva) emit sobretensionActivada();
        else emit sobretensionDesactivada();
    }
    
    // ========================================================================
    // CONVERSIÓN DIRECTA: ADC → UNIDADES FÍSICAS
    // ========================================================================
    
    tensionFiltrada   = adc_tension   * FACTOR_SENSIBILIDAD_TENSION;    // V
    
    // ════ CALIBRACIÓN DINÁMICA DE CORRIENTE SEGÚN CARGAS ACTIVAS ════
    corrienteFiltrada = adc_corriente  * FACTOR_SENSIBILIDAD_CORRIENTE;
    // ═══════════════════════════════════════════════════════════════
    
    // Presión: usar la clase PressureConverter para conversión completa

    presionBar = pressureConverter.convertirPresionBar(adc_presion);  // Presión en bar
    presionPa  = pressureConverter.convertirPresionPa(presionBar);

    alturaFiltrada =pressureConverter.convertirAlturaCm(presionBar);

    // Aceleraciones: conversión directa con factor
    aceleracionX = (float)accel_x_raw * FACTOR_SENSIBILIDAD_ACCEL;  // g
    aceleracionY = (float)accel_y_raw * FACTOR_SENSIBILIDAD_ACCEL;  // g
    
    // Angulo: Conversion a partir de la aceleracion en X e Y
    anguloGrados = atan2(aceleracionY, aceleracionX) * 180.0 /M_PI;

    // Potencia: producto directo
    potenciaCalculada = tensionFiltrada * corrienteFiltrada;  // W
    
    datosValidos = true;
    
    // Log opcional (comentar en producción)
    ui->textEditLog->append(QString("T:%1V I:%2A P:%3W H:%4cm RPM:%5")
                                .arg(QString::number(adc_tension))
                                .arg(QString::number(adc_corriente))
                                .arg(potenciaCalculada, 0, 'f', 2)
                                .arg(QString::number(adc_presion))
                                .arg(QString::number(rpmActual))
                            );

    
    // Sincronizar estados de relés (siempre, en cada trama)
    sincronizarEstadosCargas(estado_reles);

    // Bloquear control manual de cargas durante alerta por sobretensión
    bool permitirControlCargas = conectado() && !alertaSobretensionActiva;
    ui->pushButtonR1->setEnabled(permitirControlCargas);
    ui->pushButtonR2->setEnabled(permitirControlCargas);
    ui->pushButtonR3->setEnabled(permitirControlCargas);
    ui->pushButtonR4->setEnabled(permitirControlCargas);

    // Capturar datos filtrados en recepción si estamos grabando
    if (capturandoMediciones) {
        // Tiempo de muestra basado en periodo fijo de adquisición (10 ms)
        // Se usa índice de muestra para evitar deriva de reloj del sistema
        qint64 ms = static_cast<qint64>(datosMediciones.size() - 1) * PERIODO_MUESTREO_MS;

        // Formato: Tiempo(ms);Tension(V);Corriente(A);Potencia(W);Presion(Bar);Altura(cm);RPM;Angulo(deg)
        QString linea = QString("%1;%2;%3;%4;%5;%6;%7;%8")
                .arg(ms)
                .arg(tensionFiltrada, 0, 'f', 1)
                .arg(corrienteFiltrada, 0, 'f', 3)
                .arg(potenciaCalculada, 0, 'f', 2)
                .arg(presionBar, 0, 'f', 4)
                .arg(alturaFiltrada, 0, 'f', 0)
                .arg(rpmActual)
            .arg(anguloGrados, 0, 'f', 0);

        datosMediciones.append(linea);
        
        // Actualizar LCD Number con cantidad de muestras (sin contar el encabezado)
        int cantidadMuestras = datosMediciones.size() - 1;
        ui->lcdNumber->display(cantidadMuestras);

        // Detención automática al alcanzar el máximo de muestras
        if (cantidadMuestras >= MAX_MUESTRAS_MEDICION) {
            ui->textEditLog->append(
                QString("Captura detenida automaticamente: se alcanzaron %1 muestras")
                    .arg(MAX_MUESTRAS_MEDICION));
            ui->pushButtonMedicion->setChecked(false);
        }
    }
}


void MainWindow::sincronizarEstadosCargas(uint8_t estadoDigital)
{
    // Extraer estado de cada carga (bits 0-3)
    // LÓGICA: bit=1→ON, bit=0→OFF (consistente con TX)
    bool nuevoEstado0 = (estadoDigital & 0x01) != 0;  // Carga 1
    bool nuevoEstado1 = (estadoDigital & 0x02) != 0;  // Carga 2
    bool nuevoEstado2 = (estadoDigital & 0x04) != 0;  // Carga 3
    bool nuevoEstado3 = (estadoDigital & 0x08) != 0;  // Carga 4
    
    // SIEMPRE actualizar desde ESP32 (fuente de verdad del hardware)
    // Esto garantiza que la UI refleje el estado REAL del hardware
    // incluso si hubo errores, protecciones o comandos rechazados
    
    estadoRele0 = nuevoEstado0;
    actualizarEstadoBotonRele(ui->pushButtonR1, estadoRele0);
    
    estadoRele1 = nuevoEstado1;
    actualizarEstadoBotonRele(ui->pushButtonR2, estadoRele1);
    
    estadoRele2 = nuevoEstado2;
    actualizarEstadoBotonRele(ui->pushButtonR3, estadoRele2);
    
    estadoRele3 = nuevoEstado3;
    actualizarEstadoBotonRele(ui->pushButtonR4, estadoRele3);
}

void MainWindow::actualizarLCD()
{
    // ========================================================================
    // SIN CONEXIÓN O SIN DATOS - PLACEHOLDERS CON FORMATO ELEGIDO
    // ========================================================================
    if (!conectado() || !datosValidos) {
        ui->edit_lcd_1->setText("   UTN-UNDIMOTRIZ   ");
        ui->edit_lcd_2->setText("   --- V   ---- RPM ");
        ui->edit_lcd_3->setText("  ---- A    --- DEG ");
        ui->edit_lcd_4->setText("  ---- W    --- CM  ");
        return;
    }
    
    // ========================================================================
    // CONECTADO Y CON DATOS VÁLIDOS - MOSTRAR VALORES REALES
    // ========================================================================
    
    // Línea 1: Título centrado (siempre igual)
    ui->edit_lcd_1->setText("   UTN-UNDIMOTRIZ   ");
    
    // Línea 2: Tensión (V) | RPM (Velocidad)
    // Formato: "  230.9 V  2500 rpm " (20 chars)
    QString linea2 = QString("%1 V  %2 RPM ")
        .arg(tensionFiltrada, 6, 'f', 1)     // Ancho 6, 1 decimal
        .arg(rpmActual, 4);                  // Ancho 4, sin decimal (directo)
    ui->edit_lcd_2->setText(linea2);
    
    // Línea 3: Corriente (A) | Ángulo (deg)
    // Formato: "  1.234 A    45 deg" (20 chars)
    QString linea3 = QString("%1 A   %2 DEG ")
        .arg(corrienteFiltrada, 6, 'f', 3)   // Ancho 6, 3 decimales
        .arg(anguloGrados, 3, 'f', 0);       // Ancho 3, sin decimal (con atan2)
    ui->edit_lcd_3->setText(linea3);
    
    // Línea 4: Potencia (W) | Altura (cm)
    // Formato: "  28.46 W   101 cm  " (20 chars)
    QString linea4 = QString("%1 W   %2 CM  ")
        .arg(potenciaCalculada, 6, 'f', 2)   // Ancho 6, 2 decimales
        .arg(alturaFiltrada, 3, 'f', 0);    // Ancho 3, sin decimal (altura en cm)
    ui->edit_lcd_4->setText(linea4);
}

void MainWindow::on_datosRecibidos()
{
    QByteArray bytesRx = tcpSocket->readAll(); // Lee todos los datos disponibles
    
    // Comentado para producción - logs de recepción deshabilitados
    // ui->textEditLog->append(QString("DEBUG: Recibidos %1 bytes").arg(bytesRx.size()));
    
    datosRecibidos.append(bytesRx);

    procesarDatosRecibidos();
}

void MainWindow::procesarDatosRecibidos()
{
    for (int i = 0; i < datosRecibidos.size(); i++) {
        unsigned char dato = datosRecibidos.at(i);
        
        switch(estadoRecepcion) {
        case ESPERO_INICIO_TRAMA:
            if(dato == TRAMA_INICIO) {  // 0xAA
                tramaBuffer.clear();
                tramaBuffer.append(dato);
                contadorBytes = 1;
                estadoRecepcion = RECIBIENDO_DATOS;
            }
            break;
            
        case RECIBIENDO_DATOS:
            tramaBuffer.append(dato);
            contadorBytes++;
            
            // Cuando llegamos a 15 bytes completos
            if(contadorBytes == TRAMA_TOTAL_BYTES) {
                // Validar byte de fin en posición [14] (0x55)
                if((unsigned char)tramaBuffer[14] == TRAMA_FIN) {
                    // Trama válida - procesar
                    procesarTramaTelemetria();
                    // ui->textEditRecibido->append("RX: OK");
                } else {
                    // Byte de fin incorrecto
                    ui->textEditLog->append(QString("ERROR: Byte[14]=0x%1 (esperado 0x55)")
                                           .arg((unsigned char)tramaBuffer[14], 2, 16, QChar('0')));
                }
                
                // Reiniciar para próxima trama
                estadoRecepcion = ESPERO_INICIO_TRAMA;
                contadorBytes = 0;
            }
            else if(contadorBytes > TRAMA_TOTAL_BYTES) {
                // Protección: desbordamiento (no debería pasar)
                ui->textEditLog->append("ERROR: Desbordamiento de buffer");
                estadoRecepcion = ESPERO_INICIO_TRAMA;
                contadorBytes = 0;
            }
            break;
        }
    }
    datosRecibidos.clear();
}


void MainWindow::on_pushButtonConectar_clicked()
{
    //Si no hay conexión activa, creamos una nueva
    if (!tcpSocket) {
        //Obtenemos la dirección del servidor del comboBox
        QString serverAddress = "192.168.4.1:80"; // Agreger aca el ip
        if (serverAddress.isEmpty()) {
            QMessageBox::critical(this, QString::fromLatin1("Error de conexión"), QString::fromLatin1("Ingrese una dirección de servidor válida"));
            return;
        }
        
        // Parsear la dirección IP:Puerto
        QString host;
        quint16 port;
        if (!parsearDireccionServidor(serverAddress, host, port)) {
            QMessageBox::critical(this, "Error de conexión", "Formato incorrecto. Use: IP:Puerto (ej: 192.168.4.1:80)");
            return;
        }
        
        //Creamos el socket TCP
        tcpSocket = new QTcpSocket(this);
        
        //Conectamos las señales que nos interesen
        connect(tcpSocket, &QTcpSocket::readyRead, this, &MainWindow::on_datosRecibidos);
        connect(tcpSocket, &QTcpSocket::connected, this, &MainWindow::actualizarEstadoConexion);
        connect(tcpSocket, &QTcpSocket::disconnected, this, &MainWindow::actualizarEstadoConexion);
        
        //Intentamos conectar al servidor
        ui->textEditLog->append(QString("Conectando a %1:%2").arg(host).arg(port));
        
        tcpSocket->connectToHost(host, port);
        
        // Esperamos un tiempo razonable para la conexión
        if (tcpSocket->waitForConnected(5000)) {
            ui->textEditLog->append("Conectado a " + serverAddress);
            
            // Ya no necesitamos flag primeraTramaRecibida - estados vienen en cada trama
        } else {
            // Si hubo un error en la conexión...
            QMessageBox::critical(this, "Error", "No se puede conectar a " + serverAddress + "\nError: " + tcpSocket->errorString());
            delete tcpSocket;
            tcpSocket = nullptr;
        }
    }
    else {
        //Si había una conexión activa, la cerramos
        tcpSocket->disconnectFromHost();
        if (!tcpSocket->waitForDisconnected(3000)) {
            tcpSocket->abort();
        }
        delete tcpSocket;
        tcpSocket = nullptr;
        ui->textEditLog->append("Desconectado del servidor");
    }

    actualizarEstadoConexion();
}

void MainWindow::habilitarAreaMediciones(bool habilitar)
{
    if (habilitar) {
        // Conectado - habilitar todo el área de mediciones
        ui->groupBoxMEDICIONES->setEnabled(true);
        
        // Habilitar/deshabilitar botones de cargas según alerta de sobretensión
        bool permitirControlCargas = !alertaSobretensionActiva;
        ui->pushButtonR1->setEnabled(permitirControlCargas);
        ui->pushButtonR2->setEnabled(permitirControlCargas);
        ui->pushButtonR3->setEnabled(permitirControlCargas);
        ui->pushButtonR4->setEnabled(permitirControlCargas);
        ui->pushButtonMedicion->setEnabled(true);
        // Restablecer opacidad normal
        ui->groupBoxMEDICIONES->setStyleSheet("QGroupBox { opacity: 1.0; }");
        
        // Los valores reales del LCD se actualizarán automáticamente con el timer
        
    } else {
        // Desconectado - deshabilitar botones de cargas y mostrar valores por defecto
        ui->pushButtonR1->setEnabled(false);
        ui->pushButtonR2->setEnabled(false);
        ui->pushButtonR3->setEnabled(false);
        ui->pushButtonR4->setEnabled(false);
        ui->pushButtonMedicion->setEnabled(false);
        // Hacer el área más opaca
        ui->groupBoxMEDICIONES->setStyleSheet("QGroupBox { opacity: 0.6; }");
        
        // Mostrar solo etiquetas y unidades cuando no hay conexión (formato Propuesta 3)
        ui->edit_lcd_1->setText("   UTN-UNDIMOTRIZ   ");
        ui->edit_lcd_2->setText("   --- V   ---- RPM ");
        ui->edit_lcd_3->setText("  ---- A    --- DEG ");
        ui->edit_lcd_4->setText("  ---- W    --- CM  ");
        
        // Resetear datos de mediciones
        datosValidos = false;
        tensionFiltrada = 0.0;
        corrienteFiltrada = 0.0;
        potenciaCalculada = 0.0;
        
        // Resetear nuevos sensores
        alturaFiltrada = 0.0;
        rpmActual = 0;
        aceleracionX = 0.0;
        aceleracionY = 0.0;
        
        // Resetear estados de cargas y botones
        estadoRele0 = estadoRele1 = estadoRele2 = estadoRele3 = false;
        alertaSobretensionActiva = false;
        actualizarEstadoBotonRele(ui->pushButtonR1, false);  // CARGA 1
        actualizarEstadoBotonRele(ui->pushButtonR2, false);  // CARGA 2
        actualizarEstadoBotonRele(ui->pushButtonR3, false);  // CARGA 3
        actualizarEstadoBotonRele(ui->pushButtonR4, false);  // CARGA 4
        
        // Flag primeraTramaRecibida ya no se usa con trama de 15 bytes
    }
}

void MainWindow::enviarEstadosReles()
{
    if (!conectado()) {
        ui->textEditLog->append("ERROR: No hay conexión TCP para enviar comando de carga");
        return;
    }
    
    // ========================================================================
    // CALCULAR BYTE DE ESTADOS DESDE VARIABLES LOCALES
    // LÓGICA: bit=1→ON, bit=0→OFF (misma convención que en recepción)
    // ========================================================================
    uint8_t byteEstados = 0x00;  // Empezamos con todos OFF (bits en 0)
    
    // Poner bit en 1 para cada relé que esté ON
    if (estadoRele0) byteEstados |= 0x01;  // bit0 = Relé 1
    if (estadoRele1) byteEstados |= 0x02;  // bit1 = Relé 2
    if (estadoRele2) byteEstados |= 0x04;  // bit2 = Relé 3
    if (estadoRele3) byteEstados |= 0x08;  // bit3 = Relé 4
    
    // ========================================================================
    // CONSTRUIR TRAMA BINARIA DE 3 BYTES
    // ========================================================================
    QByteArray trama;
    trama.append(static_cast<char>(0xAA));          // Byte 0: Inicio
    trama.append(static_cast<char>(byteEstados));   // Byte 1: Estados
    trama.append(static_cast<char>(0x55));          // Byte 2: Fin
    
    // Enviar por TCP
    tcpSocket->write(trama);
    tcpSocket->flush();
    
    // Log en hexadecimal para debug
    ui->textEditRecibido->append(
        QString("TX: 0x%1 0x%2 0x%3")
        .arg((uint8_t)trama[0], 2, 16, QChar('0').toUpper())
        .arg((uint8_t)trama[1], 2, 16, QChar('0').toUpper())
        .arg((uint8_t)trama[2], 2, 16, QChar('0').toUpper())
    );
    
    // Log adicional con interpretación binaria (opcional)
    // ui->textEditLog->append(QString("Estados: %1 (0b%2)")
    //     .arg(byteEstados, 2, 16, QChar('0'))
    //     .arg(byteEstados, 8, 2, QChar('0')));
}

void MainWindow::actualizarEstadoBotonRele(QPushButton* boton, bool estado)
{
    // Extraer el número de la carga del nombre del objeto
    QString nombreBoton = boton->objectName();
    QString numeroCarga = nombreBoton.right(1); // Tomar el último carácter (1, 2, 3, 4)
    
    // Los botones mantienen su estilo original, no se cambian
    // El texto ahora es CARGA en lugar de RELE
    int numeroDisplay = numeroCarga.toInt(); // Ya son 1, 2, 3, 4
    boton->setText("CARGA " + QString::number(numeroDisplay));
    
    // Actualizar el LineEdit correspondiente
    QLineEdit* lineEdit = nullptr;
    if (numeroCarga == "1") lineEdit = ui->lineEditR1;
    else if (numeroCarga == "2") lineEdit = ui->lineEditR2;
    else if (numeroCarga == "3") lineEdit = ui->lineEditR3;
    else if (numeroCarga == "4") lineEdit = ui->lineEditR4;
    
    if (lineEdit) {
        if (estado) {
            // Carga activada - mismo estilo que conexión exitosa
            lineEdit->setStyleSheet("font-weight: bold; color: black; background-color: lightgreen; text-align: center;");
            lineEdit->setText("ON");
        } else {
            // Carga desactivada - mismo estilo que desconectado
            lineEdit->setStyleSheet("font-weight: normal; color: white; background-color: red; text-align: center;");
            lineEdit->setText("OFF");
        }
        // Asegurar que el texto esté centrado
        lineEdit->setAlignment(Qt::AlignCenter);
    }
}

// Slots para botones de cargas (implementación con toggle)
void MainWindow::on_pushButtonR1_clicked()
{
    // Solo funciona si está conectado
    if (!conectado()) return;
    if (alertaSobretensionActiva) {
        ui->textEditLog->append("Control bloqueado: sobretension activa.");
        return;
    }
    
    // Toggle del estado (estadoRele0 representa CARGA 1)
    estadoRele0 = !estadoRele0;
    
    // Enviar estado de TODOS los relés (protocolo binario de 3 bytes)
    enviarEstadosReles();
    
    // NO actualizar UI aquí - esperar confirmación desde ESP32
    // La actualización se hará en sincronizarEstadosCargas() cuando llegue la trama RX
}

void MainWindow::on_pushButtonR2_clicked()
{
    // Solo funciona si está conectado
    if (!conectado()) return;
    if (alertaSobretensionActiva) {
        ui->textEditLog->append("Control bloqueado: sobretension activa.");
        return;
    }
    
    // Toggle del estado (estadoRele1 representa CARGA 2)
    estadoRele1 = !estadoRele1;
    
    // Enviar estado de TODOS los relés (protocolo binario de 3 bytes)
    enviarEstadosReles();
    
    // NO actualizar UI aquí - esperar confirmación desde ESP32
    // La actualización se hará en sincronizarEstadosCargas() cuando llegue la trama RX
}

void MainWindow::on_pushButtonR3_clicked()
{
    // Solo funciona si está conectado
    if (!conectado()) return;
    if (alertaSobretensionActiva) {
        ui->textEditLog->append("Control bloqueado: sobretension activa.");
        return;
    }
    
    // Toggle del estado (estadoRele2 representa CARGA 3)
    estadoRele2 = !estadoRele2;
    
    // Enviar estado de TODOS los relés (protocolo binario de 3 bytes)
    enviarEstadosReles();
    
    // NO actualizar UI aquí - esperar confirmación desde ESP32
    // La actualización se hará en sincronizarEstadosCargas() cuando llegue la trama RX
}

void MainWindow::on_pushButtonR4_clicked()
{
    // Solo funciona si está conectado
    if (!conectado()) return;
    if (alertaSobretensionActiva) {
        ui->textEditLog->append("Control bloqueado: sobretension activa.");
        return;
    }
    
    // Toggle del estado (estadoRele3 representa CARGA 4)
    estadoRele3 = !estadoRele3;
    
    // Enviar estado de TODOS los relés (protocolo binario de 3 bytes)
    enviarEstadosReles();
    
    // NO actualizar UI aquí - esperar confirmación desde ESP32
    // La actualización se hará en sincronizarEstadosCargas() cuando llegue la trama RX
}

void MainWindow::on_pushButtonMedicion_toggled(bool checked)
{
    if (checked) {
        // Empezar medición
        ui->pushButtonMedicion->setText("Detener Grabacion");
        ui->pushButtonMedicion->setIcon(QIcon(":/stop.png"));
        ui->lcdNumber->display(0);
        ui->lcdNumber->setStyleSheet("color: lightgreen; background-color: black; border: 2px solid lightgreen;");
        capturandoMediciones = true;
        datosMediciones.clear();
        // Añadir encabezado CSV (separador ';' para compatibilidad Excel locales)
        datosMediciones.append("Tiempo(ms);Tension(V);Corriente(A);Potencia(W);Presion(Bar);Altura(cm);RPM;Angulo(deg)");
        tiempoInicioMediciones = 0;


        ui->textEditLog->append(
            QString("Iniciada captura de mediciones (periodo %1 ms, maximo %2 muestras)")
                .arg(PERIODO_MUESTREO_MS)
                .arg(MAX_MUESTRAS_MEDICION));
    } else {
        // Detener y guardar
        ui->pushButtonMedicion->setText("Iniciar Grabacion");
        ui->pushButtonMedicion->setIcon(QIcon(":/play.png"));
        capturandoMediciones = false;
        ui->lcdNumber->setStyleSheet("color: red; background-color: black; border: 2px solid red;");
        guardarMedicionesCSV();
    }
}

void MainWindow::onSobretensionActivada()
{
    ui->textEditLog->append("ALERTA: Sobretension activa. Control manual de cargas bloqueado.");
    actualizarEstadoConexion();
}

void MainWindow::onSobretensionDesactivada()
{
    ui->textEditLog->append("ALERTA: Sobretension desactivada. Control manual de cargas habilitado.");
    actualizarEstadoConexion();
}

void MainWindow::guardarMedicionesCSV()
{
    // Debe existir al menos 1 muestra además del encabezado
    if (datosMediciones.size() <= 1) {
        ui->textEditLog->append("No hay muestras para guardar.");
        return;
    }

    const QString carpetaDocumentos =
        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    const QString nombreSugerido =
        QString("medicion_%1.csv").arg(QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss"));
    const QString rutaSugerida = QDir(carpetaDocumentos).filePath(nombreSugerido);

    QString rutaArchivo = QFileDialog::getSaveFileName(
        this,
        "Guardar medicion",
        rutaSugerida,
        "CSV (*.csv)");

    // Usuario canceló el diálogo
    if (rutaArchivo.isEmpty()) {
        ui->textEditLog->append("Guardado cancelado por el usuario.");
        return;
    }

    // Asegurar extensión .csv
    if (!rutaArchivo.endsWith(".csv", Qt::CaseInsensitive)) {
        rutaArchivo += ".csv";
    }

    QFile archivo(rutaArchivo);
    if (!archivo.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        QMessageBox::critical(this,
                             "Error al guardar",
                             "No se pudo crear el archivo:\n" + rutaArchivo +
                                 "\n\nDetalle: " + archivo.errorString());
        ui->textEditLog->append("ERROR: No se pudo guardar el archivo CSV");
        return;
    }

    // BOM UTF-8 para mejor compatibilidad con Excel en Windows
    archivo.write("\xEF\xBB\xBF");

    QTextStream stream(&archivo);
    for (const QString &linea : datosMediciones) {
        stream << linea << "\n";
    }
    archivo.close();

    ui->textEditLog->append(
        QString("Medicion guardada en: %1 | Muestras: %2")
            .arg(QDir::toNativeSeparators(rutaArchivo))
            .arg(datosMediciones.size() - 1));
}

