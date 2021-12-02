######## Reconocimiento de Mascarilla en Tiempo Real #########

# Importar librerías
import os #Funciones básicas del sistema operativo
import cv2 #OpenCV para la visión artificial
import numpy as np #Para los cálculos y funciones
import tensorflow as tf #Para el aprendizaje automático
import sys #Provee acceso a algunas funciones o variables usadas o mantenidas por el intérprete

# Esto es necesario debido a que el código se encuentra dentro de la carpeta object_detection
sys.path.append("..")

# Importar utilidades
from utils import label_map_util #Para las etiquetas
from utils import visualization_utils as vis_util #Para la visualización

# Nombre del directorio que contiene el modelo
MODEL_NAME = 'inference_graph'

# Define el path como el directorio de trabajo actual
CWD_PATH = os.getcwd()

# Define el path completo del modelo que se utilizará para detectar objetos
PATH_TO_CKPT = os.path.join(CWD_PATH,MODEL_NAME,'frozen_inference_graph.pb')

# Define el path completo con el nombre de las clases
PATH_TO_LABELS = os.path.join(CWD_PATH,'training','labelmap.pbtxt')

# Número de clases de los objetos que se pueden identificar
NUM_CLASSES = 2

# Carga el label map que indica los nombres de las clases
label_map = label_map_util.load_labelmap(PATH_TO_LABELS)
categories = label_map_util.convert_label_map_to_categories(label_map, max_num_classes=NUM_CLASSES, use_display_name=True)
category_index = label_map_util.create_category_index(categories)

# Carga el modelo Tensorflow en la memoria
detection_graph = tf.Graph()
with detection_graph.as_default():
    od_graph_def = tf.GraphDef()
    with tf.gfile.GFile(PATH_TO_CKPT, 'rb') as fid:
        serialized_graph = fid.read()
        od_graph_def.ParseFromString(serialized_graph)
        tf.import_graph_def(od_graph_def, name='')

    sess = tf.Session(graph=detection_graph)

# Definir tensores de entrada y salida para la detección de objeto
# El tensor de entrada es la imagen
image_tensor = detection_graph.get_tensor_by_name('image_tensor:0')

# El tensor de salida son los boxes, el puntaje y las clases
# Cada box representa una parte de la imagen donde un objeto en particular es detectado
detection_boxes = detection_graph.get_tensor_by_name('detection_boxes:0')

# Cada puntaje representa la certeza del objeto detectado
# El puntaje es mostrado en la imagen resultante junto a la clase
detection_scores = detection_graph.get_tensor_by_name('detection_scores:0')
detection_classes = detection_graph.get_tensor_by_name('detection_classes:0')

# Número de objetos detectados
num_detections = detection_graph.get_tensor_by_name('num_detections:0')

# Inicializa la cámara
video = cv2.VideoCapture(0)
ret = video.set(1,1920)
ret = video.set(1,1080)

while(True):

    # Acquirir el frame en base al video
    ret, frame = video.read()
    frame_expanded = np.expand_dims(frame, axis=0)

    # Realiza la detección ejecutando el modelo con la imagen como entrada
    (boxes, scores, classes, num) = sess.run(
        [detection_boxes, detection_scores, detection_classes, num_detections],
        feed_dict={image_tensor: frame_expanded})

    # Dibuja los resultados de la detección
    vis_util.visualize_boxes_and_labels_on_image_array(
        frame,
        np.squeeze(boxes),
        np.squeeze(classes).astype(np.int32),
        np.squeeze(scores),
        category_index,
        use_normalized_coordinates=True,
        line_thickness=8,
        min_score_thresh=0.85)

    # Una vez ya dibujado el resultado en el frame, mostrarlo en pantalla
    cv2.imshow('S.D.M.V.A - Webcam', frame)

    # Presionar 'q' para salir
    if cv2.waitKey(1) == ord('q'):
        break

# Cierra la ventana
video.release()
cv2.destroyAllWindows()

